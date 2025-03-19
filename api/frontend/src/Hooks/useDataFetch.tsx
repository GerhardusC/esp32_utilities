import { useEffect, useReducer, useState } from "react";

import { ActionTypes, temperatureAndHumidityInitialState, temperatureAndHumidityReducer, TemperatureAndHumidityAction, FetchingStatus, TemperatureAndHumidityState } from "../Contexts/TemperatureAndHumidityContext";
import { getNowEpoch } from "../utils/funcs";

interface HandleDataFetchParams {
    dataFetchDispatch: React.ActionDispatch<[action: TemperatureAndHumidityAction]>;
    timestamp?: number;
    startStop?: [number, number];
}

const handleDataFetch = async (params: HandleDataFetchParams) => {
    const { dataFetchDispatch, startStop, timestamp } = params;

    dataFetchDispatch({
        type: ActionTypes.SET_FETCHING_STATUS,
        payload: {
            fetchingStatus: FetchingStatus.PENDING,
        }
    })
    const endpoint = startStop ?  
        `/get_data_in_range?start=${startStop[0]}&stop=${startStop[1]}`:
        `/get_data_since?timestamp=${timestamp ?? getNowEpoch() - 3600}`;
    try {
        const res = await fetch(endpoint);
        if(res.ok){
            dataFetchDispatch({
                type: ActionTypes.SET_FETCHING_STATUS,
                payload: {
                    fetchingStatus: FetchingStatus.SUCCESS,
                }
            })
            const data = await res.json();
            dataFetchDispatch({
                type: ActionTypes.UPDATE_DATA,
                payload: {
                    data,
                }
            })
        }
    } catch {
        dataFetchDispatch({
            type: ActionTypes.SET_FETCHING_STATUS,
            payload: {
                fetchingStatus: FetchingStatus.FAIL,
            }
        })
    }
}

const useDataFetch = () => {
    const [heartbeat, setHeartbeat] = useState(false);
    const [dataFetchState, dataFetchDispatch] = useReducer(temperatureAndHumidityReducer, temperatureAndHumidityInitialState);
    useEffect(() => {
        if(dataFetchState.data){
            dataFetchDispatch({
                type: ActionTypes.UPDATE_DATA,
                payload: {
                    data: dataFetchState.data,
                }
            })
        }
        console.log("FetchingStatus updated to: ", dataFetchState.fetchingStatus);
    }, [dataFetchState.fetchingStatus]);

    useEffect(() => {
        handleDataFetch({
            dataFetchDispatch,
            startStop: dataFetchState.startStop ?? undefined,
            timestamp: dataFetchState.sinceTimestamp ?? undefined,
        });
        console.log("Heartbeat updated to: ", heartbeat);
    }, [heartbeat]);

    useEffect(() => {
        if(dataFetchState.sinceTimestamp === null){
            return;
        }
        const intervalDur = getIntervalDurationFromTimestamp(dataFetchState.sinceTimestamp);
        console.log("Interval dur:", intervalDur);

        setHeartbeat(prev => !prev);
        const interval = setInterval(() => {
            setHeartbeat(prev => !prev);
        }, intervalDur);

        return () => {
            clearInterval(interval);
        }

    }, [dataFetchState.sinceTimestamp]);
    
    useEffect(() => {
        if(dataFetchState.startStop === null){
            return;
        }
        setHeartbeat(prev => !prev);

    }, [dataFetchState.startStop]);

    return [dataFetchState, dataFetchDispatch] as [TemperatureAndHumidityState,  React.ActionDispatch<[action: TemperatureAndHumidityAction]>];
}

export default useDataFetch;

const getIntervalDurationFromTimestamp = (timestamp: number) => {
    const intervalSize = ((getNowEpoch() - timestamp) / 1000)*1000;
    if(intervalSize > 10000){
        return 10000;
    }
    if(intervalSize > 50000){
        return 50000;
    }
    return intervalSize;
}