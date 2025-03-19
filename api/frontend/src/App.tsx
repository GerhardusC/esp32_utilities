import './App.css'
import DataFetchDateSelection from './Components/SpecificComponents/DataFetchDateSelection';
import TempHumGraph from './Components/SpecificComponents/TempHumGraph'
import { TemperatureAndHumidityContext } from './Contexts/TemperatureAndHumidityContext'
import useDataFetch from './Hooks/useDataFetch'

function App() {
    const [dataFetchState, dataFetchDispatch] = useDataFetch();
    return (
        <>
            <TemperatureAndHumidityContext.Provider value={[dataFetchState, dataFetchDispatch]}>
                <DataFetchDateSelection />
                <TempHumGraph />
            </TemperatureAndHumidityContext.Provider>
        </>
    )
}

export default App
