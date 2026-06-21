pub type DefaultPrecision = f64;

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum TokenType {
    Skip,
    Number,
    Operator,
    Parentheses,
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum ColorTokenType {
    Empty,
    Number,
    Operator,
    Function,
    Parentheses,
    Brackets,
    CurlyBrackets,
    Variables,
    Comma,
}

#[derive(Clone, Copy)]
pub struct CalcToken {
    pub typ: TokenType,
    pub val: DefaultPrecision,
    pub op: char,
    pub depth: u32,
    pub start: usize,
    pub end: usize,
}

impl Default for CalcToken {
    fn default() -> Self {
        Self {
            typ: TokenType::Skip,
            val: 0.0,
            op: '\0',
            depth: 0,
            start: 0,
            end: 0,
        }
    }
}

#[derive(Clone, Copy)]
pub struct ColorToken {
    pub typ: ColorTokenType,
    pub start: usize,
}

pub const BUFFER_SIZE: usize = 2049;
pub const RESULT_SIZE: usize = 1025;
pub const USER_MISTAKE: i32 = 2;
pub const CODE_MISTAKE: i32 = 1;
pub const WELCOME: &str = "Welcome to the realtime CLI math tool!";
pub const VALID_LIST: &str = "0123456789+-*/().^% ";
pub const OPERATIONS: &str = "+-*/^%";
pub const DELIMITERS: &str = ".()[]+-*/^%,{}";
pub const VERSION: &str = "release 1.28.8";

pub const CT_FLAG_EMPTY: u8 = 0;
pub const CT_FLAG_READ_BRACKETS: u8 = 1;
pub const CT_FLAG_READ_CURLY_BRACKETS: u8 = 2;
pub const CT_FLAG_READ_COMMAS: u8 = 4;

pub const USE_PRETTY_COLORS: u8 = 1;

pub const RESET: &str = "\u{1b}[0m";
pub const BOLD: &str = "\u{1b}[1m";
#[allow(dead_code)]
pub const DIM: &str = "\u{1b}[2m";
pub const ITALIC: &str = "\u{1b}[3m";
pub const UNDERLINE: &str = "\u{1b}[4m";
pub const RED: &str = "\u{1b}[31m";
pub const GREEN: &str = "\u{1b}[32m";
pub const YELLOW: &str = "\u{1b}[33m";
pub const BLUE: &str = "\u{1b}[34m";
pub const MAGENTA: &str = "\u{1b}[35m";
#[allow(dead_code)]
pub const CYAN: &str = "\u{1b}[36m";
#[allow(dead_code)]
pub const BLACK: &str = "\u{1b}[37m";

pub const ERROR_CLR: &str = RED;
pub const NUMBER_CLR: &str = YELLOW;
pub const OPERATOR_CLR: &str = MAGENTA;
pub const FUNCTION_CLR: &str = BLUE;
pub const BRACKETS_CLR: &str = GREEN;
pub const PAREN_CLR: &str = BLUE;
pub const CURLY_BRACKETS_CLR: &str = RED;
pub const VARIABLE_CLR: &str = YELLOW;
pub const COMMA_CLR: &str = GREEN;

pub const MAX_VARIABLES: usize = 32;

#[derive(Clone)]
pub struct Variable {
    pub value: DefaultPrecision,
    pub name: String,
}

#[derive(Clone, Copy, PartialEq, Eq)]
pub enum FuncIndex {
    SquareRoot,
    CubeRoot,
    Sine,
    Cosine,
    Tangent,
    SineH,
    CosineH,
    TangentH,
    SineR,
    CosineR,
    TangentR,
    SineRH,
    CosineRH,
    TangentRH,
    Cosecant,
    Secant,
    Cotangent,
    CosecantH,
    SecantH,
    CotangentH,
    Floor,
    Ceiling,
    NLog,
    DLog,
    BLog,
    Gamma,
    Truncate,
    ErrorFunc,
    ErrorFuncC,
    LGamma,
    Absolute,
    Round,
    RadToDeg,
    DegToRad,
    ExponentE,
    Exponent2,
    MultiArgPadding,
    PowerFunc,
    Maximum,
    Minimum,
    Hypotenuse,
    XLog,
    TangentA2,
}

impl FuncIndex {
    pub fn is_multi_arg(&self) -> bool {
        (*self as u32) > (FuncIndex::MultiArgPadding as u32)
    }
}

#[derive(Clone, Copy, PartialEq, Eq, Debug)]
pub enum ErrorCode {
    SeeminglyOkay,
    InvalidChar,
    InsufficientCloseParen,
    InsufficientOpenParen,
    EmptyParen,
    InvalidOperand,
    InvalidOperator,
    InsufficientNums,
    InsufficientOps,
    FuncInvalidBrackets,
    EmptyFunction,
    VarOpenBrackets,
    VarUnknown,
    MultiArgInsufficient,
    MultiArgExcess,
    MultiArgInvalidFirst,
    MultiArgInvalidSecond,
    DisplaySizeLimit,
    InputSizeLimit,
}

impl ErrorCode {
    pub fn is_ok(&self) -> bool {
        matches!(self, ErrorCode::SeeminglyOkay)
    }
}
