use crate::args::ARGS;
use crate::database;
use crate::error::Error;
use axum::{
    extract::Query,
    http::StatusCode,
    response::IntoResponse,
    routing::{delete, get, get_service},
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
            "/get_all_data_from_device",
            get(fetch_all_device_data_handler),
        )
        .route(
            "/get_data_in_range_from_device",
            get(fetch_device_data_in_range_handler),
        )
        .route(
            "/api/delete_all_data_before_timestamp",
            delete(delete_before_timestamp_handler),
        )
}

pub fn routes_static() -> Router {
    Router::new().nest_service("/", get_service(ServeDir::new(&ARGS.frontend_directory)))
}

/**
 * Route handlers:
 */
#[axum::debug_handler]
async fn fetch_device_data_in_range_handler(
    Query(params): Query<database::QueryDataFromDeviceInRange>,
) -> Result<impl IntoResponse, Error> {
    let res = database::read_device_data_in_range(params, &ARGS.db_path);
    match res {
        Ok(result) => Ok(Json(result)),
        Err(e) => Err(Error::from(e)),
    }
}

#[axum::debug_handler]
async fn fetch_all_device_data_handler(
    Query(params): Query<database::QueryAllDataFromDevice>,
) -> Result<impl IntoResponse, Error> {
    let res = database::read_device_data(&params.device_id, &ARGS.db_path);
    match res {
        Ok(result) => Ok(Json(result)),
        Err(e) => Err(Error::from(e)),
    }
}

#[derive(Deserialize)]
struct DeleteFromDeviceBeforeTimestampRequestParams {
    timestamp: u64,
    device_id: String,
}

#[derive(Serialize)]
struct DeleteConfirmation {
    items_deleted: usize,
}

#[axum::debug_handler]
async fn delete_before_timestamp_handler(
    Json(delete_before): Json<DeleteFromDeviceBeforeTimestampRequestParams>,
) -> Result<impl IntoResponse, Error> {
    let lines_changed = database::delete_device_data_before(
        delete_before.timestamp,
        &delete_before.device_id,
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
