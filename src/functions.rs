use crate::definitions::*;
use crate::utilities::*;

fn parse_number_prefix(s: &str, pos: usize) -> Option<(DefaultPrecision, usize)> {
    let bytes = s.as_bytes();
    let start = pos;
    let mut i = pos;
    let mut has_digit = false;

    if i < bytes.len() && (bytes[i] == b'+' || bytes[i] == b'-') {
        i += 1;
    }

    while i < bytes.len() && bytes[i].is_ascii_digit() {
        i += 1;
        has_digit = true;
    }

    if i < bytes.len() && bytes[i] == b'.' {
        i += 1;
        while i < bytes.len() && bytes[i].is_ascii_digit() {
            i += 1;
            has_digit = true;
        }
    }

    if !has_digit || i == start {
        return None;
    }

    s[start..i].parse::<f64>().ok().map(|v| (v, i))
}

#[allow(unused_assignments)]
pub fn validate_buffer(
    buf: &str,
    highest_prio: Option<&mut u8>,
    variables: &[Variable],
) -> ErrorCode {
    let bytes = buf.as_bytes();
    let mut pos = 0;
    let mut open_parens: i64 = 0;
    let mut last_was_paren = false;
    let mut nums: usize = 0;
    let mut ops: usize = 0;
    let mut mode = false;
    let mut hp_local: u8 = 0;

    while pos < bytes.len() {
        pos = skip_whitespace(buf, pos);
        if pos >= bytes.len() {
            break;
        }
        last_was_paren = false;

        if !mode {
            if bytes[pos] == b'{' {
                let close = match find_var_close(buf, pos) {
                    Some(c) => c,
                    None => return ErrorCode::VarOpenBrackets,
                };
                let var_name = &buf[pos + 1..close];
                let fail = !variables.iter().any(|v| v.name == var_name);
                if fail {
                    return ErrorCode::VarUnknown;
                }
                pos = close + 1;
                nums += 1;
                mode = !mode;
                continue;
            }

            if let Some(index) = get_func_index(&buf[pos..]) {
                if index.is_multi_arg() {
                    let (open, close) = match find_func_close(buf, pos) {
                        Ok(oc) => oc,
                        Err(_) => return ErrorCode::FuncInvalidBrackets,
                    };

                    let mut is_empty = true;
                    for b in &bytes[open + 1..close] {
                        if !(*b as char).is_ascii_whitespace() {
                            is_empty = false;
                            break;
                        }
                    }
                    if is_empty {
                        return ErrorCode::EmptyFunction;
                    }

                    let comma = match find_func_comma(buf, open + 1, close) {
                        Some(c) => c,
                        None => return ErrorCode::MultiArgInsufficient,
                    };
                    if find_func_comma(buf, comma + 1, close).is_some() {
                        return ErrorCode::MultiArgExcess;
                    }

                    let mut child_check = skip_whitespace(buf, open + 1);
                    if child_check == comma {
                        return ErrorCode::MultiArgInvalidFirst;
                    }
                    child_check = skip_whitespace(buf, comma + 1);
                    if child_check == close {
                        return ErrorCode::MultiArgInvalidSecond;
                    }

                    let child_one = &buf[open + 1..comma];
                    let ret = validate_buffer(child_one, None, variables);
                    if !ret.is_ok() {
                        return ret;
                    }

                    let child_two = &buf[comma + 1..close];
                    let ret = validate_buffer(child_two, None, variables);
                    if !ret.is_ok() {
                        return ret;
                    }

                    pos = close + 1;
                    nums += 1;
                    mode = !mode;
                    continue;
                }

                let (open, close) = match find_func_close(buf, pos) {
                    Ok(oc) => oc,
                    Err(_) => return ErrorCode::FuncInvalidBrackets,
                };

                let mut is_empty = true;
                for b in &bytes[open + 1..close] {
                    if !(*b as char).is_ascii_whitespace() {
                        is_empty = false;
                        break;
                    }
                }
                if is_empty {
                    return ErrorCode::EmptyFunction;
                }

                let child = &buf[open + 1..close];
                let ret = validate_buffer(child, None, variables);
                if !ret.is_ok() {
                    return ret;
                }

                pos = close + 1;
                nums += 1;
                mode = !mode;
                continue;
            }
        }

        if !VALID_LIST.as_bytes().contains(&bytes[pos]) {
            return ErrorCode::InvalidChar;
        }

        match bytes[pos] {
            b'(' => {
                if last_was_paren {
                    return ErrorCode::EmptyParen;
                }
                last_was_paren = true;
                open_parens += 1;
                pos += 1;
                continue;
            }
            b')' => {
                if last_was_paren {
                    return ErrorCode::EmptyParen;
                }
                if open_parens > 0 {
                    open_parens -= 1;
                } else {
                    return ErrorCode::InsufficientOpenParen;
                }
                pos += 1;
                continue;
            }
            _ => {}
        }

        if !mode {
            nums += 1;
            match parse_number_prefix(buf, pos) {
                Some((_, new_pos)) => {
                    pos = new_pos;
                }
                None => return ErrorCode::InvalidOperand,
            }
        } else {
            ops += 1;
            if !OPERATIONS.as_bytes().contains(&bytes[pos]) {
                return ErrorCode::InvalidOperator;
            }
            let prio = get_priority(bytes[pos] as char);
            if prio > hp_local {
                hp_local = prio;
            }
            pos += 1;
        }

        mode = !mode;
    }

    if nums > ops + 1 {
        return ErrorCode::InsufficientOps;
    } else if ops > nums.saturating_sub(1) {
        return ErrorCode::InsufficientNums;
    }

    if open_parens > 0 {
        return ErrorCode::InsufficientCloseParen;
    }
    if let Some(hp) = highest_prio {
        *hp = hp_local;
    }
    ErrorCode::SeeminglyOkay
}

pub fn count_tokens(buf: &str, flags: u8) -> usize {
    let bytes = buf.as_bytes();
    let mut count = 0;
    let mut pos = 0;
    let mut mode = false;

    while pos < bytes.len() {
        pos = skip_whitespace(buf, pos);
        if pos >= bytes.len() {
            break;
        }

        if !mode {
            if get_func_index(&buf[pos..]).is_some() {
                if flags & CT_FLAG_READ_BRACKETS != 0 {
                    // keep ptr where it is for bracket reading
                } else {
                    let close = find_func_close(buf, pos).map(|(_, c)| c).unwrap_or(pos);
                    pos = close + 1;
                    mode = !mode;
                }
                count += 1;
                continue;
            }
        }

        if flags & CT_FLAG_READ_COMMAS != 0 && bytes[pos] == b',' {
            pos += 1;
            count += 1;
            mode = !mode;
            continue;
        }

        if bytes[pos] == b'(' || bytes[pos] == b')' {
            pos += 1;
            count += 1;
            continue;
        }

        if flags & CT_FLAG_READ_CURLY_BRACKETS != 0 && bytes[pos] == b'{' {
            let close = find_var_close(buf, pos).unwrap_or(pos);
            pos = close + 1;
            count += 3;
            continue;
        }

        if flags & CT_FLAG_READ_BRACKETS != 0 && (bytes[pos] == b'[' || bytes[pos] == b']') {
            pos += 1;
            count += 1;
            continue;
        }

        if !mode {
            if let Some((_, new_pos)) = parse_number_prefix(buf, pos) {
                pos = new_pos;
            } else {
                pos += 1;
            }
        } else {
            pos += 1;
        }

        count += 1;
        mode = !mode;
    }

    count
}

pub fn calculate_buffer(
    buf: &str,
    highest_prio: u8,
    variables: &[Variable],
) -> DefaultPrecision {
    let count = count_tokens(buf, CT_FLAG_EMPTY);
    let mut tokens: Vec<CalcToken> = vec![CalcToken::default(); count];

    let bytes = buf.as_bytes();
    let mut pos = 0;
    let mut j = 0;
    let mut mode = false;
    let mut paren_level = 0;

    while pos < bytes.len() {
        pos = skip_whitespace(buf, pos);
        if pos >= bytes.len() {
            break;
        }

        if !mode {
            if bytes[pos] == b'{' {
                let close = find_var_close(buf, pos).unwrap();
                let var_name = &buf[pos + 1..close];
                for v in variables.iter() {
                    if v.name == var_name {
                        tokens[j].val = v.value;
                        tokens[j].typ = TokenType::Number;
                        break;
                    }
                }
                pos = close + 1;
                j += 1;
                mode = !mode;
                continue;
            }

            if let Some(index) = get_func_index(&buf[pos..]) {
                if index.is_multi_arg() {
                    let (open, close) = find_func_close(buf, pos).unwrap();
                    let comma = find_func_comma(buf, open + 1, close).unwrap();

                    let child_one = &buf[open + 1..comma];
                    let child_one_prio = get_buf_highest_prio(child_one);
                    let child_two = &buf[comma + 1..close];
                    let child_two_prio = get_buf_highest_prio(child_two);

                    tokens[j].typ = TokenType::Number;
                    tokens[j].val = match index {
                        FuncIndex::XLog => calculate_buffer(child_two, child_two_prio, variables).log(calculate_buffer(child_one, child_one_prio, variables)),
                        FuncIndex::Hypotenuse => calculate_buffer(child_one, child_one_prio, variables).hypot(calculate_buffer(child_two, child_two_prio, variables)),
                        FuncIndex::TangentA2 => calculate_buffer(child_one, child_one_prio, variables).atan2(calculate_buffer(child_two, child_two_prio, variables)),
                        FuncIndex::Maximum => calculate_buffer(child_one, child_one_prio, variables).max(calculate_buffer(child_two, child_two_prio, variables)),
                        FuncIndex::Minimum => calculate_buffer(child_one, child_one_prio, variables).min(calculate_buffer(child_two, child_two_prio, variables)),
                        FuncIndex::PowerFunc => calculate_buffer(child_one, child_one_prio, variables).powf(calculate_buffer(child_two, child_two_prio, variables)),
                        _ => 0.0,
                    };

                    pos = close + 1;
                    j += 1;
                    mode = !mode;
                    continue;
                }

                let (open, close) = find_func_close(buf, pos).unwrap();
                let child = &buf[open + 1..close];
                let child_prio = get_buf_highest_prio(child);

                tokens[j].typ = TokenType::Number;
                let cv = calculate_buffer(child, child_prio, variables);
                tokens[j].val = apply_func(index, cv);

                pos = close + 1;
                j += 1;
                mode = !mode;
                continue;
            }
        }

        if bytes[pos] == b'(' || bytes[pos] == b')' {
            tokens[j].typ = TokenType::Parentheses;
            tokens[j].op = bytes[pos] as char;
            tokens[j].start = pos;
            match bytes[pos] {
                b'(' => {
                    paren_level += 1;
                    tokens[j].depth = paren_level;
                }
                b')' => {
                    tokens[j].depth = paren_level;
                    paren_level -= 1;
                }
                _ => {}
            }
            pos += 1;
            j += 1;
            continue;
        }

        if !mode {
            if let Some((val, new_pos)) = parse_number_prefix(buf, pos) {
                tokens[j].typ = TokenType::Number;
                tokens[j].val = val;
                tokens[j].start = pos;
                tokens[j].end = new_pos;
                pos = new_pos;
                j += 1;
            }
        } else {
            tokens[j].typ = TokenType::Operator;
            tokens[j].op = bytes[pos] as char;
            pos += 1;
            j += 1;
        }

        if pos >= bytes.len() {
            break;
        }
        mode = !mode;
    }

    // compress parentheses
    for i in 0..count {
        if tokens[i].typ == TokenType::Parentheses && tokens[i].op == '(' {
            let start = tokens[i].start + 1;
            let s_depth = tokens[i].depth;
            let mut end = start;
            let mut end_idx = i;

            for l in (i + 1)..count {
                let found_prio = get_priority(tokens[l].op);
                if found_prio > 0 && tokens[l].typ == TokenType::Operator {
                    // track child priority
                }
                if tokens[l].typ == TokenType::Parentheses && tokens[l].depth == s_depth && tokens[l].op == ')' {
                    end = tokens[l].start;
                    end_idx = l;
                    break;
                }
            }

            let child = &buf[start..end];
            let mut child_prio: u8 = 0;
            for l in (i + 1)..end_idx {
                if tokens[l].typ == TokenType::Operator {
                    let p = get_priority(tokens[l].op);
                    if p > child_prio {
                        child_prio = p;
                    }
                }
            }

            tokens[i].typ = TokenType::Number;
            tokens[i].val = calculate_buffer(child, child_prio, variables);
            for l in (i + 1)..=end_idx {
                tokens[l].typ = TokenType::Skip;
            }
        }
    }

    // reduce by priority
    for prio in (1..=highest_prio).rev() {
        for i in 0..count {
            if tokens[i].typ != TokenType::Operator
                || get_priority(tokens[i].op) != prio
                || tokens[i].typ == TokenType::Skip
            {
                continue;
            }

            let mut left = 0.0;
            let mut lidx = 0;
            for j in (0..i).rev() {
                if tokens[j].typ == TokenType::Number {
                    left = tokens[j].val;
                    lidx = j;
                    break;
                }
            }

            let mut right = 0.0;
            let mut ridx = 0;
            for j in (i + 1)..count {
                if tokens[j].typ == TokenType::Number {
                    right = tokens[j].val;
                    ridx = j;
                    break;
                }
            }

            tokens[i].val = calculate_trio(left, tokens[i].op, right);
            tokens[i].typ = TokenType::Number;
            tokens[lidx].typ = TokenType::Skip;
            tokens[ridx].typ = TokenType::Skip;
        }
    }

    for i in 0..count {
        if tokens[i].typ == TokenType::Number {
            return tokens[i].val;
        }
    }
    0.0
}

fn get_buf_highest_prio(buf: &str) -> u8 {
    let bytes = buf.as_bytes();
    let mut highest: u8 = 0;
    for &b in bytes {
        let c = b as char;
        if OPERATIONS.contains(c) {
            let p = get_priority(c);
            if p > highest {
                highest = p;
            }
        }
    }
    highest
}

fn apply_func(index: FuncIndex, val: DefaultPrecision) -> DefaultPrecision {
    match index {
        FuncIndex::SquareRoot => val.sqrt(),
        FuncIndex::CubeRoot => val.cbrt(),
        FuncIndex::Sine => val.sin(),
        FuncIndex::Cosine => val.cos(),
        FuncIndex::Tangent => val.tan(),
        FuncIndex::SineH => val.sinh(),
        FuncIndex::CosineH => val.cosh(),
        FuncIndex::TangentH => val.tanh(),
        FuncIndex::SineR => val.asin(),
        FuncIndex::CosineR => val.acos(),
        FuncIndex::TangentR => val.atan(),
        FuncIndex::SineRH => val.asinh(),
        FuncIndex::CosineRH => val.acosh(),
        FuncIndex::TangentRH => val.atanh(),
        FuncIndex::NLog => val.ln(),
        FuncIndex::DLog => val.log10(),
        FuncIndex::BLog => val.log2(),
        FuncIndex::Floor => val.floor(),
        FuncIndex::Ceiling => val.ceil(),
        FuncIndex::Gamma => gamma(val),
        FuncIndex::LGamma => lgamma(val),
        FuncIndex::Truncate => val.trunc(),
        FuncIndex::ErrorFunc => erf(val),
        FuncIndex::ErrorFuncC => erfc(val),
        FuncIndex::Absolute => val.abs(),
        FuncIndex::Round => val.round(),
        FuncIndex::ExponentE => val.exp(),
        FuncIndex::Exponent2 => val.exp2(),
        FuncIndex::Cosecant => 1.0 / val.sin(),
        FuncIndex::Secant => 1.0 / val.cos(),
        FuncIndex::Cotangent => 1.0 / val.tan(),
        FuncIndex::CosecantH => 1.0 / val.sinh(),
        FuncIndex::SecantH => 1.0 / val.cosh(),
        FuncIndex::CotangentH => 1.0 / val.tanh(),
        FuncIndex::RadToDeg => val / std::f64::consts::PI * 180.0,
        FuncIndex::DegToRad => val / 180.0 * std::f64::consts::PI,
        _ => 0.0,
    }
}

fn gamma(x: f64) -> f64 {
    libm::tgamma(x)
}

fn lgamma(x: f64) -> f64 {
    libm::lgamma_r(x).0
}

fn erf(x: f64) -> f64 {
    libm::erf(x)
}

fn erfc(x: f64) -> f64 {
    libm::erfc(x)
}

pub fn print_buf_colored(buf: &str) {
    let count = count_tokens(
        buf,
        CT_FLAG_READ_BRACKETS | CT_FLAG_READ_CURLY_BRACKETS | CT_FLAG_READ_COMMAS,
    );
    let mut tokens: Vec<ColorToken> = vec![ColorToken { typ: ColorTokenType::Empty, start: 0 }; count + 1];

    let bytes = buf.as_bytes();
    let mut pos = 0;
    let mut j = 0;
    let mut mode = false;

    while pos < bytes.len() && j < count {
        pos = skip_whitespace(buf, pos);
        if pos >= bytes.len() {
            break;
        }

        if !mode && get_func_index(&buf[pos..]).is_some() {
            tokens[j].typ = ColorTokenType::Function;
            tokens[j].start = pos;
            pos = buf[pos..].find('[').map(|p| pos + p).unwrap_or(pos);
            j += 1;
            continue;
        }

        if bytes[pos] == b',' {
            tokens[j].typ = ColorTokenType::Comma;
            tokens[j].start = pos;
            mode = !mode;
            pos += 1;
            j += 1;
            continue;
        }

        if bytes[pos] == b'(' || bytes[pos] == b')' {
            tokens[j].typ = ColorTokenType::Parentheses;
            tokens[j].start = pos;
            mode = !mode;
            pos += 1;
            j += 1;
            continue;
        }

        if bytes[pos] == b'{' {
            tokens[j].typ = ColorTokenType::CurlyBrackets;
            tokens[j].start = pos;
            j += 1;

            let close = find_var_close(buf, pos).unwrap_or(pos);
            tokens[j].typ = ColorTokenType::Variables;
            tokens[j].start = pos + 1;
            j += 1;

            tokens[j].typ = ColorTokenType::CurlyBrackets;
            tokens[j].start = close;
            pos = close + 1;
            j += 1;

            mode = !mode;
            continue;
        }

        if bytes[pos] == b'[' || bytes[pos] == b']' {
            tokens[j].typ = ColorTokenType::Brackets;
            tokens[j].start = pos;
            pos += 1;
            j += 1;
            continue;
        }

        if !mode {
            tokens[j].typ = ColorTokenType::Number;
            tokens[j].start = pos;
            if let Some((_, new_pos)) = parse_number_prefix(buf, pos) {
                pos = new_pos;
            } else {
                pos += 1;
            }
            j += 1;
        } else {
            tokens[j].typ = ColorTokenType::Operator;
            tokens[j].start = pos;
            pos += 1;
            j += 1;
        }

        if pos >= bytes.len() {
            break;
        }
        mode = !mode;
    }

    tokens[j].typ = ColorTokenType::Empty;
    tokens[j].start = pos;

    use std::io::Write;
    let stdout = std::io::stdout();
    let mut out = stdout.lock();

    for i in 0..count {
        if tokens[i].typ == ColorTokenType::Empty {
            break;
        }

        let clr = match tokens[i].typ {
            ColorTokenType::Number => NUMBER_CLR,
            ColorTokenType::Operator => OPERATOR_CLR,
            ColorTokenType::Parentheses => PAREN_CLR,
            ColorTokenType::Brackets => BRACKETS_CLR,
            ColorTokenType::Function => FUNCTION_CLR,
            ColorTokenType::CurlyBrackets => CURLY_BRACKETS_CLR,
            ColorTokenType::Variables => VARIABLE_CLR,
            ColorTokenType::Comma => COMMA_CLR,
            _ => RESET,
        };

        let next_start = tokens[i + 1].start;
        let len = next_start - tokens[i].start;
        let _ = write!(out, "{}{}", clr, &buf[tokens[i].start..tokens[i].start + len]);
    }
    let _ = write!(out, "{}", RESET);
}

#[cfg(test)]
mod tests {
    use super::*;

    fn calc(buf: &str) -> f64 {
        let mut hp: u8 = 0;
        let vars: Vec<Variable> = vec![];
        let ret = validate_buffer(buf, Some(&mut hp), &vars);
        assert!(ret.is_ok(), "validation failed for '{}': {:?}", buf, ret);
        calculate_buffer(buf, hp, &vars)
    }

    fn calc_with_vars(buf: &str, vars: &[Variable]) -> f64 {
        let mut hp: u8 = 0;
        let ret = validate_buffer(buf, Some(&mut hp), vars);
        assert!(ret.is_ok(), "validation failed for '{}': {:?}", buf, ret);
        calculate_buffer(buf, hp, vars)
    }

    fn validate(buf: &str) -> ErrorCode {
        let mut hp: u8 = 0;
        validate_buffer(buf, Some(&mut hp), &[])
    }

    #[test]
    fn test_basic_arithmetic() {
        assert!((calc("2+2") - 4.0).abs() < 1e-10);
        assert!((calc("10-3") - 7.0).abs() < 1e-10);
        assert!((calc("4*5") - 20.0).abs() < 1e-10);
        assert!((calc("20/4") - 5.0).abs() < 1e-10);
        assert!((calc("7%3") - 1.0).abs() < 1e-10);
    }

    #[test]
    fn test_precedence() {
        assert!((calc("2+3*4") - 14.0).abs() < 1e-10);
        assert!((calc("2*3+4") - 10.0).abs() < 1e-10);
        assert!((calc("2^3+1") - 9.0).abs() < 1e-10);
        assert!((calc("2+3^2") - 11.0).abs() < 1e-10);
    }

    #[test]
    fn test_parentheses() {
        assert!((calc("(2+3)*4") - 20.0).abs() < 1e-10);
        assert!((calc("2*(3+4)") - 14.0).abs() < 1e-10);
        assert!((calc("((1+2))") - 3.0).abs() < 1e-10);
        assert!((calc("(2+3)*(4+5)") - 45.0).abs() < 1e-10);
    }

    #[test]
    fn test_nested_parens() {
        assert!((calc("2*(3+(4*5))") - 46.0).abs() < 1e-10);
        assert!((calc("((2+3)*4)+1") - 21.0).abs() < 1e-10);
    }

    #[test]
    fn test_decimals() {
        assert!((calc("3.14+1") - 4.14).abs() < 1e-10);
        assert!((calc(".5*4") - 2.0).abs() < 1e-10);
        assert!((calc("2.5*2") - 5.0).abs() < 1e-10);
    }

    #[test]
    fn test_functions() {
        assert!((calc("sqrt[16]") - 4.0).abs() < 1e-10);
        assert!((calc("cbrt[27]") - 3.0).abs() < 1e-10);
        assert!((calc("abs[-5]") - 5.0).abs() < 1e-10);
        assert!((calc("floor[3.7]") - 3.0).abs() < 1e-10);
        assert!((calc("ceil[3.2]") - 4.0).abs() < 1e-10);
        assert!((calc("round[3.5]") - 4.0).abs() < 1e-10);
        assert!((calc("trunc[3.9]") - 3.0).abs() < 1e-10);
    }

    #[test]
    fn test_trig() {
        assert!((calc("sin[0]") - 0.0).abs() < 1e-10);
        assert!((calc("cos[0]") - 1.0).abs() < 1e-10);
        assert!((calc("exp[0]") - 1.0).abs() < 1e-10);
        assert!((calc("log[1]") - 0.0).abs() < 1e-10);
        assert!((calc("log10[100]") - 2.0).abs() < 1e-10);
        assert!((calc("log2[8]") - 3.0).abs() < 1e-10);
    }

    #[test]
    fn test_multi_arg() {
        assert!((calc("pow[2,3]") - 8.0).abs() < 1e-10);
        assert!((calc("fmax[3,7]") - 7.0).abs() < 1e-10);
        assert!((calc("fmin[3,7]") - 3.0).abs() < 1e-10);
        assert!((calc("logx[2,8]") - 3.0).abs() < 1e-10);
        assert!((calc("hypot[3,4]") - 5.0).abs() < 1e-10);
    }

    #[test]
    fn test_nested_functions() {
        assert!((calc("sqrt[sqrt[16]]") - 2.0).abs() < 1e-10);
        assert!((calc("abs[0-sqrt[9]]") - 3.0).abs() < 1e-10);
        assert!((calc("pow[2,sqrt[9]]") - 8.0).abs() < 1e-10);
    }

    #[test]
    fn test_func_with_expression() {
        assert!((calc("sqrt[9+7]") - 4.0).abs() < 1e-10);
        assert!((calc("pow[1+1,2+1]") - 8.0).abs() < 1e-10);
        assert!((calc("sin[0]+cos[0]") - 1.0).abs() < 1e-10);
    }

    #[test]
    fn test_variables() {
        let vars = vec![Variable {
            value: 42.0,
            name: "x".to_string(),
        }];
        assert!((calc_with_vars("{x}", &vars) - 42.0).abs() < 1e-10);
        assert!((calc_with_vars("{x}+8", &vars) - 50.0).abs() < 1e-10);
        assert!((calc_with_vars("{x}*2", &vars) - 84.0).abs() < 1e-10);
        assert!((calc_with_vars("sqrt[{x}+7]", &vars) - 7.0).abs() < 1e-10);
    }

    #[test]
    fn test_whitespace() {
        assert!((calc("  2  +  2  ") - 4.0).abs() < 1e-10);
        assert!((calc("2 + 2") - 4.0).abs() < 1e-10);
    }

    #[test]
    fn test_error_invalid_char() {
        assert_eq!(validate("2#a"), ErrorCode::InvalidChar);
    }

    #[test]
    fn test_error_unclosed_paren() {
        assert_eq!(validate("(2+2"), ErrorCode::InsufficientCloseParen);
    }

    #[test]
    fn test_error_unopened_paren() {
        assert_eq!(validate("2+2)"), ErrorCode::InsufficientOpenParen);
    }

    #[test]
    fn test_error_missing_operand() {
        assert_eq!(validate("2+"), ErrorCode::InsufficientNums);
    }

    #[test]
    fn test_error_missing_operator() {
        assert_eq!(validate("2 2"), ErrorCode::InvalidOperator);
    }

    #[test]
    fn test_error_invalid_operator() {
        assert_eq!(validate("2&3"), ErrorCode::InvalidChar);
    }

    #[test]
    fn test_error_unknown_variable() {
        assert_eq!(validate("{y}"), ErrorCode::VarUnknown);
    }

    #[test]
    fn test_error_unclosed_variable() {
        assert_eq!(validate("{x"), ErrorCode::VarOpenBrackets);
    }

    #[test]
    fn test_error_func_invalid_brackets() {
        assert_eq!(validate("sqrt(16)"), ErrorCode::FuncInvalidBrackets);
    }

    #[test]
    fn test_error_empty_function() {
        assert_eq!(validate("sqrt[   ]"), ErrorCode::EmptyFunction);
    }

    #[test]
    fn test_error_multi_arg_insufficient() {
        assert_eq!(validate("pow[2]"), ErrorCode::MultiArgInsufficient);
    }

    #[test]
    fn test_error_multi_arg_excess() {
        assert_eq!(validate("pow[2,3,4]"), ErrorCode::MultiArgExcess);
    }
}
