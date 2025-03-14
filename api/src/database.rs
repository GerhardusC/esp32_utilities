use crate::error::Error;
use rusqlite::{params, Connection, Result};
use serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize)]
pub struct SensorDataPoint {
    timestamp: u64,
    topic: String,
    value: String,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct QueryDataFromDeviceInRange {
    device_id: String,
    start: u64,
    stop: u64,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct QueryAllDataFromDevice {
    pub device_id: String,
}

pub fn read_device_data(device_id: &str, db_path: &str) -> Result<Vec<SensorDataPoint>, Error> {
    let conn = Connection::open(db_path)?;
    let mut statement =
        conn.prepare("SELECT timestamp, topic, value FROM READINGS WHERE device_id = ?1;")?;

    let data_points_iter = statement.query_map(params![device_id], |row| {
        Ok(SensorDataPoint {
            timestamp: row.get(0)?,
            topic: row.get(1)?,
            value: row.get(2)?,
        })
    })?;

    let res: Vec<SensorDataPoint> = data_points_iter.collect::<rusqlite::Result<Vec<_>>>()?;
    Ok(res)
}

pub fn read_device_data_in_range(
    query: QueryDataFromDeviceInRange,
    db_path: &str,
) -> Result<Vec<SensorDataPoint>, Error> {
    let conn = Connection::open(db_path)?;
    let mut statement = conn.prepare(
        "
        SELECT * FROM READINGS
            WHERE
                    timestamp > ?1
                AND timestamp < ?2
                AND device_id = ?3;
        ",
    )?;
    let data_points_iter =
        statement.query_map(params![query.start, query.stop, query.device_id], |row| {
            Ok(SensorDataPoint {
                timestamp: row.get(0)?,
                topic: row.get(1)?,
                value: row.get(2)?,
            })
        })?;

    let res: Vec<SensorDataPoint> = data_points_iter.collect::<rusqlite::Result<Vec<_>>>()?;
    Ok(res)
}

pub fn delete_device_data_before(
    timestamp: u64,
    device_id: &str,
    db_path: &str,
) -> Result<usize, Error> {
    let conn = Connection::open(db_path)?;
    let lines_changed = conn.execute(
        "DELETE FROM READINGS WHERE timestamp < ?1 AND device_id = ?2;",
        params![timestamp, device_id],
    )?;
    Ok(lines_changed)
}
