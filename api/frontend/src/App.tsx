import './App.css'
import DataFetchDateSelection from './Components/SpecificComponents/DataFetchDateSelection';
import AllTopicsGraph from './Components/SpecificComponents/AllTopicsGraph'
import { AllTopicsContext } from './Contexts/AllTopicsContext'
import useDataFetch from './Hooks/useDataFetch'
import AllTopicsTable from './Components/SpecificComponents/AllTopicsTable';

function App() {
    const [dataFetchState, dataFetchDispatch] = useDataFetch();
    return (
        <>
            <AllTopicsContext.Provider value={[dataFetchState, dataFetchDispatch]}>
                <h1>Temperature and Humidity from DHT-11</h1>
                <DataFetchDateSelection />
                <AllTopicsGraph />
                <AllTopicsTable />
            </AllTopicsContext.Provider>
        </>
    )
}

export default App
