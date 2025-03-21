import { useContext, useState } from "react"

import DateRangePicker from "../GeneralComponents/DateRangePicker"
import SingleDateSelector from "../GeneralComponents/SingleDateSelector"
import { ActionTypes, AllTopicsContext } from "../../Contexts/AllTopicsContext";
import { tzOffsetMillis } from "../../utils/constants";
import { getNowEpoch } from "../../utils/funcs";

const DataFetchDateSelection = () => {
    const [singleDate, setSingleDate] = useState(true);
    const [dataFetchState, dataFetchDispatch] = useContext(AllTopicsContext);

    return (
        <div
            className="date-selector-outer-container"
        >
            <div
                className="date-selector-container"
            >
            {
                singleDate ?
                <SingleDateSelector
                    onOk={(timestamp) => {
                        if(!dataFetchDispatch) return;
                        dataFetchDispatch({
                            type: ActionTypes.SET_SINCE_TIMESTAMP,
                            payload: {
                                timestamp
                            }
                        })
                    }}
                    timestamp={dataFetchState.sinceTimestamp ?? getNowEpoch() - 86400}
                /> :
                <DateRangePicker
                    onOk={(startStop) => {
                        if(!dataFetchDispatch) return;
                        dataFetchDispatch({
                            type: ActionTypes.SET_START_STOP,
                            payload: {
                                startStop,
                            }
                        })
                    }}
                    startStop={
                        dataFetchState
                            .startStop?.map(item => item * 1000 - tzOffsetMillis) as [number, number] | undefined
                            ?? [new Date().valueOf() - tzOffsetMillis - 3600*1000, new Date().valueOf() - tzOffsetMillis]
                    }
                />
            }
            </div>
            <button onClick={() => {
                setSingleDate(prev => !prev);
            }}>
                {singleDate ? "Switch to range" : "Switch to timestamp"}
            </button>
        </div>
    )
}

export default DataFetchDateSelection