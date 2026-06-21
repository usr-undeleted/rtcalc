mod definitions;
mod functions;
mod utilities;

use definitions::*;
use std::io::{self, Read, Write};
use std::sync::atomic::{AtomicI32, Ordering};
use termion::raw::IntoRawMode;
use utilities::help_menu;

static RET_CODE: AtomicI32 = AtomicI32::new(0);

fn main() {
    let args: Vec<String> = std::env::args().collect();
    let mut precision: usize = 6;
    let mut prompt: &str = ">>> ";
    let mut global_flags: u8 = 0;
    let mut variables: Vec<Variable> = Vec::new();

    for i in 1..args.len() {
        let arg = &args[i];
        if arg == "help" {
            help_menu(None, 0);
        } else if arg.starts_with("prompt") {
            if let Some(eq_pos) = arg.find('=') {
                let p = &arg[eq_pos + 1..];
                if p.is_empty() {
                    help_menu(
                        Some("\n\u{1b}[31mError: Prompt not defined properly.\u{1b}[0m\n"),
                        USER_MISTAKE,
                    );
                }
                prompt = Box::leak(p.to_string().into_boxed_str());
            } else {
                help_menu(
                    Some("\n\u{1b}[31mError: Prompt not defined properly.\u{1b}[0m\n"),
                    USER_MISTAKE,
                );
            }
        } else if arg == "syntax-highlighting" {
            global_flags |= USE_PRETTY_COLORS;
        } else if arg.starts_with("precision") {
            let start = match arg.find('=') {
                Some(p) => p + 1,
                None => {
                    help_menu(
                        Some("\n\u{1b}[31mError: Precision not defined properly.\u{1b}[0m\n"),
                        USER_MISTAKE,
                    );
                }
            };
            let val = &arg[start..];
            match val.parse::<usize>() {
                Ok(p) => precision = p,
                Err(_) => help_menu(
                    Some("\n\u{1b}[31mError: Precision is an invalid number.\u{1b}[0m\n"),
                    USER_MISTAKE,
                ),
            }
        } else if arg.starts_with("define") {
            if variables.len() >= MAX_VARIABLES {
                help_menu(
                    Some("\n\u{1b}[31mError: Too many variables defined.\u{1b}[0m\n"),
                    USER_MISTAKE,
                );
            }

            let colon = match arg.find(':') {
                Some(p) => p,
                None => help_menu(
                    Some("\n\u{1b}[31mError: Definition used incorrectly.\u{1b}[0m\n"),
                    USER_MISTAKE,
                ),
            };

            let name = &arg[colon + 1..];
            if name.is_empty() {
                help_menu(
                    Some("\n\u{1b}[31mError: Definition name was not set.\u{1b}[0m\n"),
                    USER_MISTAKE,
                );
            }

            let eq_pos = match name.find('=') {
                Some(p) => p,
                None => help_menu(
                    Some("\n\u{1b}[31mError: Definition has no value.\u{1b}[0m\n"),
                    USER_MISTAKE,
                ),
            };

            if eq_pos == 0 {
                help_menu(
                    Some("\n\u{1b}[31mError: Definition name is invalid.\u{1b}[0m\n"),
                    USER_MISTAKE,
                );
            }

            let var_name = &name[..eq_pos];
            let value_str = &name[eq_pos + 1..];
            if value_str.is_empty() {
                help_menu(
                    Some("\n\u{1b}[31mError: Definition value is invalid.\u{1b}[0m\n"),
                    USER_MISTAKE,
                );
            }

            let mut child_highest_prio: u8 = 0;
            let ret = functions::validate_buffer(value_str, Some(&mut child_highest_prio), &variables);
            if !ret.is_ok() {
                help_menu(
                    Some("\n\u{1b}[31mError: Variable couldn't be expanded due to an invalid formula.\u{1b}[0m\n"),
                    USER_MISTAKE,
                );
            }

            let val = functions::calculate_buffer(value_str, child_highest_prio, &variables);
            variables.push(Variable {
                value: val,
                name: var_name.to_string(),
            });
        } else {
            help_menu(
                Some("\n\u{1b}[31mError: Improper flag used.\u{1b}[0m\n"),
                USER_MISTAKE,
            );
        }
    }

    ctrlc::set_handler(|| {
        eprintln!("\nInterrupted, exiting...");
        let code = RET_CODE.load(Ordering::SeqCst);
        std::process::exit(code);
    })
    .ok();

    let stdin = io::stdin();
    let stdout = io::stdout();
    let mut screen = match stdout.lock().into_raw_mode() {
        Ok(s) => s,
        Err(e) => {
            eprintln!("Failed to enter raw mode: {}", e);
            std::process::exit(CODE_MISTAKE);
        }
    };
    let out = screen.by_ref();

    let mut calc_buffer: Vec<u8> = Vec::with_capacity(BUFFER_SIZE);
    let mut cursor_pos: usize = 0;

    write!(out, "\n{}\u{1b}[s", prompt).unwrap();
    out.flush().unwrap();

    let mut byte_buf = [0u8; 1];

    loop {
        let mut highest_prio: u8 = 0;
        let buf_str = String::from_utf8_lossy(&calc_buffer);

        let mut ret = functions::validate_buffer(&buf_str, Some(&mut highest_prio), &variables);
        if calc_buffer.len() >= BUFFER_SIZE - 1 {
            ret = ErrorCode::InputSizeLimit;
        }

        let result = if ret.is_ok() {
            if calc_buffer.is_empty() {
                WELCOME.to_string()
            } else {
                let calc = functions::calculate_buffer(&buf_str, highest_prio, &variables);
                let formatted = format!("{:.*}", precision, calc);
                if formatted.len() >= RESULT_SIZE - 1 {
                    format!("Can't calculate: {}", utilities::error_to_str(ErrorCode::DisplaySizeLimit))
                } else {
                    formatted
                }
            }
        } else {
            format!("Can't calculate: {}", utilities::error_to_str(ret))
        };

        write!(
            out,
            "\u{1b}[u\u{1b}[J\u{1b}[A\r\u{1b}[2K{}\r\u{1b}[B{}",
            result, prompt
        )
        .unwrap();

        if global_flags & USE_PRETTY_COLORS != 0 {
            if ret.is_ok() {
                functions::print_buf_colored(&buf_str);
            } else {
                write!(out, "{}{}\u{1b}[0m", ERROR_CLR, buf_str).unwrap();
            }
        } else {
            write!(out, "{}{}\u{1b}[0m", if !ret.is_ok() { ERROR_CLR } else { "" }, buf_str).unwrap();
        }

        out.flush().unwrap();

        if ret == ErrorCode::InputSizeLimit {
            continue;
        }

        let len = calc_buffer.len();
        if len > cursor_pos {
            write!(out, "\u{1b}[{}D", len - cursor_pos).unwrap();
            out.flush().unwrap();
        }

        if stdin.lock().read(&mut byte_buf).unwrap_or(0) == 0 {
            break;
        }
        let c = byte_buf[0];

        if c == b'\n' || c == 0x0C {
            continue;
        }

        if c == 0x1B {
            let mut buf2 = [0u8; 1];
            if stdin.lock().read(&mut buf2).unwrap_or(0) == 0 {
                continue;
            }
            if buf2[0] == b'[' {
                let mut buf3 = [0u8; 1];
                if stdin.lock().read(&mut buf3).unwrap_or(0) == 0 {
                    continue;
                }

                match buf3[0] {
                    b'C' => {
                        if cursor_pos < len {
                            cursor_pos += 1;
                        }
                        continue;
                    }
                    b'D' => {
                        if cursor_pos > 0 {
                            cursor_pos -= 1;
                        }
                        continue;
                    }
                    b'1' => {
                        let mut seq = Vec::new();
                        loop {
                            let mut buf4 = [0u8; 1];
                            if stdin.lock().read(&mut buf4).unwrap_or(0) == 0 {
                                break;
                            }
                            seq.push(buf4[0]);
                            if (buf4[0] >= b'A' && buf4[0] <= b'~') || seq.len() >= 7 {
                                break;
                            }
                        }

                        let seq_str = String::from_utf8_lossy(&seq);
                        if seq_str == ";5D" {
                            move_word_left(&mut calc_buffer, &mut cursor_pos);
                        } else if seq_str == ";5C" {
                            move_word_right(&mut calc_buffer, &mut cursor_pos);
                        }
                    }
                    _ => continue,
                }
            }
            continue;
        }

        if c == 127 || c == 0x8 {
            if cursor_pos > 0 {
                calc_buffer.remove(cursor_pos - 1);
                cursor_pos -= 1;
            }
        } else if c == 0x17 {
            delete_word(&mut calc_buffer, &mut cursor_pos);
        } else if c == 0x18 {
            calc_buffer.clear();
            cursor_pos = 0;
        } else if c == 0xB {
            if cursor_pos < calc_buffer.len() {
                calc_buffer.truncate(cursor_pos);
            }
        } else if c == 0x1 {
            cursor_pos = 0;
        } else if c == 0x5 {
            cursor_pos = calc_buffer.len();
        } else if (c as char).is_ascii_graphic() || c == b' ' {
            if calc_buffer.len() < BUFFER_SIZE - 1 {
                calc_buffer.insert(cursor_pos, c);
                cursor_pos += 1;
            }
        }
    }

    write!(out, "\n").unwrap();
    out.flush().unwrap();
}

fn move_word_left(buf: &[u8], cursor_pos: &mut usize) {
    if *cursor_pos == 0 {
        return;
    }

    let is_delim = |b: u8| DELIMITERS.as_bytes().contains(&b);

    if is_delim(buf[*cursor_pos - 1]) {
        while *cursor_pos > 0 && is_delim(buf[*cursor_pos - 1]) {
            *cursor_pos -= 1;
        }
    } else {
        let mut left_white = false;
        while *cursor_pos > 0 {
            if !left_white {
                if !(buf[*cursor_pos - 1] as char).is_ascii_whitespace() {
                    left_white = true;
                }
            } else {
                if (buf[*cursor_pos - 1] as char).is_ascii_whitespace()
                    || is_delim(buf[*cursor_pos - 1])
                {
                    break;
                }
            }
            *cursor_pos -= 1;
        }
    }
}

fn move_word_right(buf: &[u8], cursor_pos: &mut usize) {
    let len = buf.len();
    if *cursor_pos >= len {
        return;
    }

    let is_delim = |b: u8| DELIMITERS.as_bytes().contains(&b);

    if is_delim(buf[*cursor_pos]) {
        while *cursor_pos < len && is_delim(buf[*cursor_pos]) {
            *cursor_pos += 1;
        }
    } else {
        let mut right_white = false;
        while *cursor_pos < len {
            if !right_white {
                if !(buf[*cursor_pos] as char).is_ascii_whitespace() {
                    right_white = true;
                }
            } else {
                if (buf[*cursor_pos] as char).is_ascii_whitespace()
                    || is_delim(buf[*cursor_pos])
                {
                    break;
                }
            }
            *cursor_pos += 1;
        }
    }
}

fn delete_word(buf: &mut Vec<u8>, cursor_pos: &mut usize) {
    if *cursor_pos == 0 {
        return;
    }

    let is_delim = |b: u8| DELIMITERS.as_bytes().contains(&b);

    if is_delim(buf[*cursor_pos - 1]) {
        while *cursor_pos > 0 && is_delim(buf[*cursor_pos - 1]) {
            buf.remove(*cursor_pos - 1);
            *cursor_pos -= 1;
        }
    } else {
        let mut left_white = false;
        while *cursor_pos > 0 {
            if !left_white {
                if !(buf[*cursor_pos - 1] as char).is_ascii_whitespace() {
                    left_white = true;
                }
            } else {
                if (buf[*cursor_pos - 1] as char).is_ascii_whitespace()
                    || is_delim(buf[*cursor_pos - 1])
                {
                    break;
                }
            }
            buf.remove(*cursor_pos - 1);
            *cursor_pos -= 1;
        }
    }
}
