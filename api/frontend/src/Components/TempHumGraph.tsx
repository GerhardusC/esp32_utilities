import { useEffect, useState } from "react";
import HighchartsReact from "highcharts-react-official";
import Highcharts from "highcharts";

type DataPoint = {
    timestamp: number,
    topic: string,
    value: string,
    device_id: string,
}

interface FetchDataBeforeHandlerArgs {
    timestamp: number,
    setHighchartsOpts: React.Dispatch<React.SetStateAction<Highcharts.Options>>,
}

const tzOffset = new Date().getTimezoneOffset()*60*1000;

const fetchDataSinceHandler = (args: FetchDataBeforeHandlerArgs) => {
    const { timestamp, setHighchartsOpts } = args;
    fetch(`/get_data_since?timestamp=${timestamp}`)
        .then(res => {
            if(res.ok) return res.json()
        })
        .then(res => {
            const result = res as DataPoint[];
            setHighchartsOpts((_prev) => {
                return {
                    series: [
                        {
                            name: "Temperature (°C)",
                            yAxis: "temp",
                            type: "spline",
                            color: "#d27c51",
                            data: result
                                .filter(item => item.topic === "/home/temperature")
                                .map(item => [item.timestamp*1000 - tzOffset, Number(item.value)])
                        },
                        {
                            name: "Humidity (%)",
                            yAxis: "hum",
                            type: "spline",
                            color: "#5c8fbb",
                            data: result
                                .filter(item => item.topic === "/home/humidity")
                                .map(item => [item.timestamp*1000 - tzOffset, Number(item.value)])
                        }
                    ]
                }
            })
        })
        .catch((err) => console.log(err));
}

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
        yAxis: [
            {
                title: {
                    text: "Temperature (°C)"
                },
                id: "temp",
            },
            {
                title: {
                    text: "Humidity (%)",
                },
                id: "hum",
            }
        ]
    });
    useEffect(() => {
        fetchDataSinceHandler({
            setHighchartsOpts,
            timestamp: Math.floor((new Date().valueOf() / 1000) - 86400)
        });
        const interval = setInterval(() => {
            fetchDataSinceHandler({
                setHighchartsOpts,
                timestamp: Math.floor((new Date().valueOf() / 1000) - 86400)
            });
        }, 10000);
        return () => {
            return clearInterval(interval);
        }

    }, [])
    return (
        <div>
            <HighchartsReact highcharts={Highcharts} options={highchartsOpts} />
        </div>
    )
}

export default TempHumGraph