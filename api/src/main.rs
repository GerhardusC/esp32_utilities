mod args;
mod database;
mod error;
mod server;

use axum::Router;
use clap::Parser;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error>> {
    args::Cli::parse();
    let app = Router::new()
        .merge(server::routes())
        .fallback_service(server::routes_static());

    let listener = tokio::net::TcpListener::bind("0.0.0.0:80").await;

    match listener {
        Ok(listener) => {
            axum::serve(listener, app).await?;
            println!("Listening on port 80");
        }
        Err(_) => {
            let listener = tokio::net::TcpListener::bind("0.0.0.0:8000").await?;
            axum::serve(listener, app).await?;
            println!("Listening on port 8000");
        }
    }

    Ok(())
}
