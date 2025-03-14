use std::time::SystemTimeError;

use axum::{http::StatusCode, response::IntoResponse};
use thiserror::Error;

#[derive(Debug, Error)]
pub enum Error {
    #[error("DB ERR")]
    DatabaseError(rusqlite::Error),
    #[error("TIMESTAMP ERR")]
    InvalidTimestampRequest(SystemTimeError),
}
impl From<rusqlite::Error> for Error {
    fn from(value: rusqlite::Error) -> Self {
        Error::DatabaseError(value)
    }
}

impl From<SystemTimeError> for Error {
    fn from(value: SystemTimeError) -> Self {
        Error::InvalidTimestampRequest(value)
    }
}

impl IntoResponse for Error {
    fn into_response(self) -> axum::response::Response {
        match self {
            Error::DatabaseError(err) => {
                return (StatusCode::INTERNAL_SERVER_ERROR, err.to_string()).into_response()
            }
            Error::InvalidTimestampRequest(err) => {
                return (StatusCode::BAD_REQUEST, err.to_string()).into_response()
            }
        };
    }
}
