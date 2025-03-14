use clap::Parser;
use std::sync::LazyLock;
/// Lightweight server that serves a sqlite database.
#[derive(Parser, Debug, Clone)]
#[command(version, about, long_about = None)]
pub struct Cli {
    /// Path to database.
    #[arg(short, long, default_value_t = String::from("./dev.db"))]
    pub db_path: String,
    /// Path to directory front end is stored in.
    #[arg(short, long, default_value_t = String::from("./dist"))]
    pub frontend_directory: String,
}

pub static ARGS: LazyLock<Cli> = LazyLock::new(|| Cli::parse());
