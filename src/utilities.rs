use crate::definitions::*;
use std::process;

pub fn help_menu(error: Option<&str>, ret: i32) -> ! {
    print!(
        "{BOLD}Real-time calculation tool (rtcalc){RESET}\n
{ITALIC}Legal and basic info:{RESET}
- Licensed under the GNU GPL-3.0 license. Open-source, and free, forever.
- Source code hosted under Github (https://github.com/usr-undeleted/rtcalc).
- Current version: {BOLD}{VERSION}{RESET}
- Made with love, by Undeleted. <3
- Functions, operators and basic syntax can be found in this program's manual.

{ITALIC}Usage:{RESET}
- This tool, of course, follows basic math principles.
{BOLD}Basic examples:{RESET}
{UNDERLINE}10 + 2 * 3{RESET}
{UNDERLINE}2 ^ (10 * (40 / 2)){RESET}

{ITALIC}Details:{RESET}
- Invalid input will lead to an error, preventing calculation.
- Input is limited to {0} characters.
- Result size is limited to {1} characters.

{ITALIC}Additional arguments:{RESET}
- {BOLD}\"help\"{RESET}: Show this menu.
- {BOLD}\"prompt=<prompt>\"{RESET}: Define a custom prompt before startup (space not included).
- {BOLD}\"syntax-highlighting\"{RESET}: Enable syntax highlighting on the input.
- {BOLD}\"precision=<num>\"{RESET}: Define what precision to show results in. Defaults to 6.
- {BOLD}\"define:<name>=<num>\"{RESET}: Create an immutable variable acessible during runtime. Definition of value may be a formula supported by the program, and may include variables defined earlier.
{2}",
        BUFFER_SIZE - 1,
        RESULT_SIZE - 1,
        error.unwrap_or("")
    );
    let _ = std::io::Write::flush(&mut std::io::stdout());
    process::exit(ret);
}

#[inline]
pub fn skip_whitespace(s: &str, mut pos: usize) -> usize {
    let bytes = s.as_bytes();
    while pos < bytes.len() && (bytes[pos] as char).is_ascii_whitespace() {
        pos += 1;
    }
    pos
}

pub fn find_func_close(s: &str, start: usize) -> Result<(usize, usize), ()> {
    let bytes = s.as_bytes();
    let open = match bytes[start..].iter().position(|&b| b == b'[') {
        Some(p) => start + p,
        None => return Err(()),
    };

    let mut depth: usize = 0;
    let mut close = open;
    let mut i = open;
    while i < bytes.len() {
        match bytes[i] {
            b'[' => depth += 1,
            b']' => {
                close = i;
                if depth > 0 {
                    depth -= 1;
                }
            }
            _ => {}
        }
        i += 1;
        if depth == 0 {
            break;
        }
    }

    if depth != 0 || close == open {
        return Err(());
    }

    Ok((open, close))
}

pub fn find_var_close(s: &str, start: usize) -> Option<usize> {
    let bytes = s.as_bytes();
    let open = match bytes[start..].iter().position(|&b| b == b'{') {
        Some(p) => start + p,
        None => return None,
    };

    let mut depth: usize = 0;
    let mut close = open;
    let mut i = open;
    while i < bytes.len() {
        match bytes[i] {
            b'{' => depth += 1,
            b'}' => {
                close = i;
                if depth > 0 {
                    depth -= 1;
                }
            }
            _ => {}
        }
        i += 1;
        if depth == 0 {
            break;
        }
    }

    if depth != 0 || close == open {
        return None;
    }

    Some(close)
}

#[inline]
pub fn get_priority(op: char) -> u8 {
    match op {
        '+' | '-' => 1,
        '*' | '/' | '%' => 2,
        '^' => 3,
        _ => 0,
    }
}

pub fn get_func_index(s: &str) -> Option<FuncIndex> {
    if s.starts_with("lgamma") { return Some(FuncIndex::LGamma); }
    if s.starts_with("atan2")  { return Some(FuncIndex::TangentA2); }
    if s.starts_with("floor")  { return Some(FuncIndex::Floor); }
    if s.starts_with("gamma")  { return Some(FuncIndex::Gamma); }
    if s.starts_with("asinh")  { return Some(FuncIndex::SineRH); }
    if s.starts_with("acosh")  { return Some(FuncIndex::CosineRH); }
    if s.starts_with("atanh")  { return Some(FuncIndex::TangentRH); }
    if s.starts_with("trunc")  { return Some(FuncIndex::Truncate); }
    if s.starts_with("log10")  { return Some(FuncIndex::DLog); }
    if s.starts_with("hypot")  { return Some(FuncIndex::Hypotenuse); }
    if s.starts_with("round")  { return Some(FuncIndex::Round); }
    if s.starts_with("log2")   { return Some(FuncIndex::BLog); }
    if s.starts_with("logx")   { return Some(FuncIndex::XLog); }
    if s.starts_with("ceil")   { return Some(FuncIndex::Ceiling); }
    if s.starts_with("sqrt")   { return Some(FuncIndex::SquareRoot); }
    if s.starts_with("cbrt")   { return Some(FuncIndex::CubeRoot); }
    if s.starts_with("sinh")   { return Some(FuncIndex::SineH); }
    if s.starts_with("cosh")   { return Some(FuncIndex::CosineH); }
    if s.starts_with("tanh")   { return Some(FuncIndex::TangentH); }
    if s.starts_with("asin")   { return Some(FuncIndex::SineR); }
    if s.starts_with("acos")   { return Some(FuncIndex::CosineR); }
    if s.starts_with("atan")   { return Some(FuncIndex::TangentR); }
    if s.starts_with("erfc")   { return Some(FuncIndex::ErrorFuncC); }
    if s.starts_with("fmax")   { return Some(FuncIndex::Maximum); }
    if s.starts_with("fmin")   { return Some(FuncIndex::Minimum); }
    if s.starts_with("csch")   { return Some(FuncIndex::CosecantH); }
    if s.starts_with("sech")   { return Some(FuncIndex::SecantH); }
    if s.starts_with("coth")   { return Some(FuncIndex::CotangentH); }
    if s.starts_with("exp2")   { return Some(FuncIndex::Exponent2); }
    if s.starts_with("pow")    { return Some(FuncIndex::PowerFunc); }
    if s.starts_with("erf")    { return Some(FuncIndex::ErrorFunc); }
    if s.starts_with("sin")    { return Some(FuncIndex::Sine); }
    if s.starts_with("cos")    { return Some(FuncIndex::Cosine); }
    if s.starts_with("tan")    { return Some(FuncIndex::Tangent); }
    if s.starts_with("log")    { return Some(FuncIndex::NLog); }
    if s.starts_with("abs")    { return Some(FuncIndex::Absolute); }
    if s.starts_with("deg")    { return Some(FuncIndex::RadToDeg); }
    if s.starts_with("rad")    { return Some(FuncIndex::DegToRad); }
    if s.starts_with("exp")    { return Some(FuncIndex::ExponentE); }
    if s.starts_with("csc")    { return Some(FuncIndex::Cosecant); }
    if s.starts_with("sec")    { return Some(FuncIndex::Secant); }
    if s.starts_with("cot")    { return Some(FuncIndex::Cotangent); }
    None
}

pub fn error_to_str(err: ErrorCode) -> &'static str {
    match err {
        ErrorCode::InvalidChar => "Invalid character in formula.",
        ErrorCode::InsufficientCloseParen => "Not enough closing parentheses.",
        ErrorCode::InsufficientOpenParen => "Not enough opening parentheses.",
        ErrorCode::InvalidOperand => "Invalid operand.",
        ErrorCode::InvalidOperator => "Invalid operator.",
        ErrorCode::InsufficientNums => "Not enough numbers for calculation",
        ErrorCode::InsufficientOps => "Not enough operators for calculation",
        ErrorCode::EmptyParen => "Empty parentheses.",
        ErrorCode::FuncInvalidBrackets => "A function has invalid brackets.",
        ErrorCode::EmptyFunction => "A function has no contents.",
        ErrorCode::VarOpenBrackets => "Invalid variable insertion, unclosed curly brackets.",
        ErrorCode::VarUnknown => "Invalid variable insertion, unknown variable.",
        ErrorCode::MultiArgInsufficient => "Not enough arguments for a multi-arg function.",
        ErrorCode::MultiArgExcess => "Too many arguments for a multi-arg function.",
        ErrorCode::MultiArgInvalidFirst => "Invalid first argument for a multi-arg function.",
        ErrorCode::MultiArgInvalidSecond => "Invalid second argument for a multi-arg function.",
        ErrorCode::DisplaySizeLimit => "Result display size limit reached - Sorry!",
        ErrorCode::InputSizeLimit => "Input size limit reached - Sorry!",
        _ => "Unknown error num - Sorry! :p",
    }
}

#[inline]
pub fn calculate_trio(left: DefaultPrecision, op: char, right: DefaultPrecision) -> DefaultPrecision {
    match op {
        '+' => left + right,
        '-' => left - right,
        '/' => left / right,
        '*' => left * right,
        '^' => left.powf(right),
        '%' => left % right,
        _ => 0.0,
    }
}

pub fn find_func_comma(s: &str, start: usize, end: usize) -> Option<usize> {
    let bytes = s.as_bytes();
    let mut depth: u32 = 0;
    let mut i = start;

    while i != end {
        match bytes[i] {
            b'(' | b'[' | b'{' => depth += 1,
            b')' | b']' | b'}' => {
                if depth > 0 {
                    depth -= 1;
                }
            }
            b',' if depth == 0 => return Some(i),
            _ => {}
        }
        i += 1;
    }

    None
}
