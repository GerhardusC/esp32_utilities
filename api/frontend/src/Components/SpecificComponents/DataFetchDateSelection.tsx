import { useContext, useState } from "react"
import DateRangePicker from "../GeneralComponents/DateRangePicker"
import SingleDateSelector from "../GeneralComponents/SingleDateSelector"
import { ActionTypes, TemperatureAndHumidityContext } from "../../Contexts/TemperatureAndHumidityContext";
import { tzOffsetMillis } from "../../utils/constants";
import { getNowEpoch } from "../../utils/funcs";

const DataFetchDateSelection = () => {
    const [singleDate, setSingleDate] = useState(true);
    const [dataFetchState, dataFetchDispatch] = useContext(TemperatureAndHumidityContext);

    return (
        <div>
            <button onClick={() => {
                setSingleDate(prev => !prev);
            }}>
                {singleDate ? "Switch to range" : "Switch to timestamp"}
            </button>
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
                    startStop={dataFetchState.startStop ?? [new Date().valueOf() - tzOffsetMillis - 3600*1000, new Date().valueOf() - tzOffsetMillis]}
                />
            }
        </div>
    )
}

export default DataFetchDateSelection