import { useContext, useEffect, useState } from "react";
import HighchartsReact from "highcharts-react-official";
import Highcharts from "highcharts";

import { topicColors, tzOffsetMillis } from "../../utils/constants";
import { AllTopicsContext } from "../../Contexts/AllTopicsContext";
import { units } from "../../utils/constants";
import { groupDataByTopic } from "../../utils/funcs";


const AllTopicsGraph = () => {
    const [highchartsOpts, setHighchartsOpts] = useState<Highcharts.Options>({
        chart: {
            zooming: {
                type: "x",
            },
            height: "30%",
        },
        title: {
            text: ""
        },
        xAxis: {
            type: "datetime"
        },
    });

    const [dataFetchState] = useContext(AllTopicsContext);

    useEffect(() => {
        const dataByTopic = groupDataByTopic(dataFetchState.data ?? []);
        console.log(dataByTopic);
        setHighchartsOpts({
            yAxis:  Object.keys(dataByTopic).map(topic => {
                const cleanedTopic = topic.replace(/\//g, "").replace("home", "");
                return {
                    title: {
                        text: `${cleanedTopic.replace(cleanedTopic[0], cleanedTopic[0].toUpperCase())} (${units.get(cleanedTopic) ?? "No unit"})`,
                    },
                    id: topic,
                }
            }),
            series: Object.keys(dataByTopic).map(topic => {
                const cleanedTopic = topic.replace(/\//g, "").replace("home", "");
                return {
                    name: cleanedTopic,
                    type: "spline",
                    data: dataByTopic[topic].map(item => [item.timestamp*1000 - tzOffsetMillis, Number(item.value)]),
                    color: topicColors.get(cleanedTopic) ?? undefined,
                    yAxis: topic,
                }
            })
            
        })
    }, [dataFetchState.data])


    return (
        <div className="chart-container">
            <HighchartsReact highcharts={Highcharts} options={highchartsOpts} />
        </div>
    )
}

export default AllTopicsGraph