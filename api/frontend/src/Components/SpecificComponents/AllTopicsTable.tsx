import { useContext } from "react";

import { AllTopicsContext } from "../../Contexts/AllTopicsContext"
import { topicColors, units } from "../../utils/constants";

const AllTopicsTable = () => {
    const [dataFetchState] = useContext(AllTopicsContext);

    if(!dataFetchState.data){
        return <h3>No data for table...</h3>
    }

    return (
        <table>
            <thead>
                <tr className="table-top-row">
                    <th>Timestamp</th>
                    <th>Category</th>
                    <th>Value</th>
                </tr>

            </thead>

            <tbody>
                {
                    dataFetchState.data.map((item, index) => {
                        const cleanedTopic = item.topic.replace("home", "").replace(/\//g, "");
                        const backgroundColorWithoutOpacity = topicColors.get(cleanedTopic) ?? "#ffffff";
                        const backgroundColor = backgroundColorWithoutOpacity + "aa";
                        return <tr key={index}
                            style={{
                                backgroundColor,
                            }}
                        >
                            <td
                            >{new Date(item.timestamp * 1000).toString()}</td>
                            <td>{cleanedTopic}</td>
                            <td>{item.value} {units.get(cleanedTopic) ?? ""}</td>
                        </tr>
                    }).reverse()
                }
            </tbody>

        </table>
    )
}

export default AllTopicsTable