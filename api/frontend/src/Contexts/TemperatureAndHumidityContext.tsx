import { createContext } from "react";

import { getNowEpoch } from "../utils/funcs";

export enum FetchingStatus {
    IDLE,
    PENDING,
    SUCCESS,
    FAIL
}

export interface TemperatureAndHumidityState {
    data: DataPoint[] | null;
    fetchingStatus: FetchingStatus;
    sinceTimestamp: number | null;
    startStop: [number, number] | null;
}

export const TemperatureAndHumidityContext = createContext<[TemperatureAndHumidityState, React.ActionDispatch<[action: TemperatureAndHumidityAction]> | null]>([{
    data: null,
    fetchingStatus: FetchingStatus.IDLE,
    sinceTimestamp: getNowEpoch() - 86400,
    startStop: null,
}, null])

export const temperatureAndHumidityInitialState: TemperatureAndHumidityState = {
    data: null,
    fetchingStatus: FetchingStatus.IDLE,
    sinceTimestamp: getNowEpoch() - 86400,
    startStop: null,
}

export enum ActionTypes {
    UPDATE_DATA,
    SET_FETCHING_STATUS,
    SET_SINCE_TIMESTAMP,
    SET_START_STOP,
}

export interface TemperatureAndHumidityPayload {
    data?: DataPoint[];
    timestamp?: number;
    fetchingStatus?: FetchingStatus;
    startStop?: [number, number];
}

export interface TemperatureAndHumidityAction {
    type: ActionTypes;
    payload: TemperatureAndHumidityPayload
}

export const temperatureAndHumidityReducer = (state: TemperatureAndHumidityState, action: TemperatureAndHumidityAction): TemperatureAndHumidityState => {
    switch (action.type) {
        case ActionTypes.UPDATE_DATA:
            return {
                ...state,
                data: action.payload.data ?? null,
            }
        case ActionTypes.SET_FETCHING_STATUS:
            return {
                ...state,
                fetchingStatus: action.payload.fetchingStatus ?? FetchingStatus.IDLE,
            }
        case ActionTypes.SET_SINCE_TIMESTAMP:
            return {
                ...state,
                startStop: null,
                sinceTimestamp: action.payload.timestamp ?? getNowEpoch() - 86400,
            }
        case ActionTypes.SET_START_STOP:
            if(!action.payload.startStop){
                const now = getNowEpoch();
                return {
                    ...state,
                    sinceTimestamp: null,
                    startStop: [now - 86400, now],
                }
            }
            let stop = action.payload.startStop[1];
            const now = getNowEpoch();
            if(stop > now){
                stop = now;
            }
            return {
                ...state,
                sinceTimestamp: null,
                startStop: [action.payload.startStop[0], stop],
            }
    }
}