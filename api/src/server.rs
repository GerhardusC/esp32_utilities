use std::{collections::HashMap, time::SystemTime};

use crate::args::ARGS;
use crate::database;
use crate::error::Error;
use axum::{
    extract::Query,
    http::StatusCode,
    response::IntoResponse,
    routing::{get, get_service},
    Json, Router,
};
use serde::{Deserialize, Serialize};
use tower_http::services::ServeDir;

/**
 * Routes:
 */
pub fn routes() -> Router {
    Router::new()
        .route(
            "/get_all_data",
            get(read_all_data_handler),
        )
        .route(
            "/get_data_since",
            get(read_data_since_handler),
        )
        .route(
            "/get_data_in_range",
            get(read_data_in_range_handler),
        )
}

pub fn routes_static() -> Router {
    Router::new().nest_service("/", get_service(ServeDir::new(&ARGS.frontend_directory)))
}

/**
 * Route handlers:
 */
#[axum::debug_handler]
async fn read_all_data_handler() -> Result<impl IntoResponse, Error> {
    let res = database::read_all_data(&ARGS.db_path);
    match res {
        Ok(result) => Ok(Json(result)),
        Err(e) => Err(Error::from(e)),
    }
}

#[axum::debug_handler]
async fn read_data_since_handler(
    Query(params): Query<HashMap<String, u64>>
) -> Result<impl IntoResponse, Error> {
    let timestamp_opt = params.get("timestamp");
    let timestamp = if let Some(x) = timestamp_opt {
        x
    } else {
        &(SystemTime::now().duration_since(SystemTime::UNIX_EPOCH)?.as_secs() - 3600)
    };
    let res = database::read_data_since(&ARGS.db_path, *timestamp);
    match res {
        Ok(result) => Ok(Json(result)),
        Err(e) => Err(Error::from(e)),
    }
}

#[axum::debug_handler]
async fn read_data_in_range_handler(
    Query(params): Query<database::QueryDataInRange>,
) -> Result<impl IntoResponse, Error> {
    let res = database::read_data_in_range(params, &ARGS.db_path);
    match res {
        Ok(result) => Ok(Json(result)),
        Err(e) => Err(Error::from(e)),
    }
}


#[derive(Deserialize)]
struct DeleteBeforeTimestampRequestParams {
    timestamp: u64,
}

#[derive(Serialize)]
struct DeleteConfirmation {
    items_deleted: usize,
}

#[axum::debug_handler]
async fn _delete_before_timestamp_handler(
    Json(delete_before): Json<DeleteBeforeTimestampRequestParams>,
) -> Result<impl IntoResponse, Error> {
    let lines_changed = database::delete_data_before(
        delete_before.timestamp,
        &ARGS.db_path,
    )?;

    if lines_changed == 0 {
        return Ok((
            StatusCode::NOT_MODIFIED,
            Json(DeleteConfirmation {
                items_deleted: lines_changed,
            }),
        ));
    }

    Ok((
        StatusCode::OK,
        Json(DeleteConfirmation {
            items_deleted: lines_changed,
        }),
    ))
}
