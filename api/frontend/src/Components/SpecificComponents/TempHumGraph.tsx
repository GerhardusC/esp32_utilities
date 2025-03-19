import { useContext, useEffect, useState } from "react";
import HighchartsReact from "highcharts-react-official";
import Highcharts from "highcharts";

import { tzOffsetMillis } from "../../utils/constants";
import { TemperatureAndHumidityContext } from "../../Contexts/TemperatureAndHumidityContext";
import { units } from "../../utils/constants";
import { groupDataByTopic } from "../../utils/funcs";


const TempHumGraph = () => {
    const [highchartsOpts, setHighchartsOpts] = useState<Highcharts.Options>({
        chart: {
            zooming: {
                type: "x",
            },
        },
        title: {
            text: "Temperature and Humidity from DHT-11"
        },
        xAxis: {
            type: "datetime"
        },
    });

    const [dataFetchState] = useContext(TemperatureAndHumidityContext);

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
                return {
                    name: topic.replace(/\//g, "").replace("home", ""),
                    type: "spline",
                    data: dataByTopic[topic].map(item => [item.timestamp*1000 - tzOffsetMillis, Number(item.value)]),
                    yAxis: topic,
                }
            })
            
        })
    }, [dataFetchState.data])


    return (
        <div>
            <HighchartsReact highcharts={Highcharts} options={highchartsOpts} />
        </div>
    )
}

export default TempHumGraph