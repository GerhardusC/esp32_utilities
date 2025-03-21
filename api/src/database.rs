use crate::error::Error;
use rusqlite::{params, Connection, Result};
use serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize)]
pub struct SensorDataPoint {
    timestamp: u64,
    topic: String,
    value: f32,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct QueryDataInRange {
    start: u64,
    stop: u64,
}

pub fn read_all_data(db_path: &str) -> Result<Vec<SensorDataPoint>, Error> {
    let conn = Connection::open(db_path)?;
    let mut statement =
        conn.prepare("SELECT timestamp, topic, value FROM READINGS;")?;

    let data_points_iter = statement.query_map([], |row| {
        Ok(SensorDataPoint {
            timestamp: row.get(0)?,
            topic: row.get(1)?,
            value: row.get(2)?,
        })
    })?;

    let res: Vec<SensorDataPoint> = data_points_iter.collect::<rusqlite::Result<Vec<_>>>()?;
    Ok(res)
}

pub fn read_data_since(db_path: &str, timestamp: u64) -> Result<Vec<SensorDataPoint>, Error> {
    let conn = Connection::open(db_path)?;
    let mut statement = conn.prepare("
        SELECT * FROM READINGS
            WHERE
                TIMESTAMP > ?1;
    ")?;
    let data_points_iter =
        statement.query_map(params![timestamp], |row| {
            Ok(SensorDataPoint {
                timestamp: row.get(0)?,
                topic: row.get(1)?,
                value: row.get(2)?,
            })
        })?;

    let res: Vec<SensorDataPoint> = data_points_iter.collect::<rusqlite::Result<Vec<_>>>()?;
    Ok(res)
}

pub fn read_data_in_range(
    query: QueryDataInRange,
    db_path: &str,
) -> Result<Vec<SensorDataPoint>, Error> {
    let conn = Connection::open(db_path)?;
    let mut statement = conn.prepare(
        "
        SELECT * FROM READINGS
            WHERE
                    timestamp > ?1
                AND timestamp < ?2;
        ",
    )?;
    let data_points_iter =
        statement.query_map(params![query.start, query.stop], |row| {
            Ok(SensorDataPoint {
                timestamp: row.get(0)?,
                topic: row.get(1)?,
                value: row.get(2)?,
            })
        })?;

    let res: Vec<SensorDataPoint> = data_points_iter.collect::<rusqlite::Result<Vec<_>>>()?;
    Ok(res)
}

pub fn delete_data_before(
    timestamp: u64,
    db_path: &str,
) -> Result<usize, Error> {
    let conn = Connection::open(db_path)?;
    let lines_changed = conn.execute(
        "DELETE FROM READINGS WHERE timestamp < ?1;",
        params![timestamp],
    )?;
    Ok(lines_changed)
}

