import { useEffect, useState } from "react";
import HighchartsReact from "highcharts-react-official";
import Highcharts from "highcharts";

type DataPoint = {
    timestamp: number,
    topic: string,
    value: string,
    device_id: string,
}

const handleDataFetch = (setHighchartsOpts: React.Dispatch<React.SetStateAction<Highcharts.Options>>) => {
    fetch("/get_all_data_from_device?device_id=DHT_11")
        .then(res => {
            if(res.ok) return res.json()
        })
        .then(res => {
            const result = res as DataPoint[];
            setHighchartsOpts((_prev) => {
                return {
                    series: [
                        {
                            name: "Temperature",
                            type: "spline",
                            data: result.filter(item => item.topic === "/home/temperature").map(item => [item.timestamp, Number(item.value.split(" ")[0].replace(",", "."))])
                        },
                        {
                            name: "Humidity",
                            type: "spline",
                            data: result.filter(item => item.topic === "/home/humidity").map(item => [item.timestamp, Number(item.value.split(" ")[0].replace(",", "."))])
                        }
                    ]
                }
            })
        })
        .catch((err) => alert(err));
}

const TempHumGraph = () => {
    const [highchartsOpts, setHighchartsOpts] = useState<Highcharts.Options>({});
    useEffect(() => {
        handleDataFetch(setHighchartsOpts);
        const interval = setInterval(() => {
            handleDataFetch(setHighchartsOpts);
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