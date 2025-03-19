import { tzOffsetMillis } from "./constants"

export const getNowEpoch = (): number => {
    return Math.floor((new Date().valueOf() - tzOffsetMillis) / 1000)
}

type DataPointByTopic = {
    [key: string]: DataPoint[],
}

export const groupDataByTopic = (data: DataPoint[]): DataPointByTopic => {
    const dataByTopic: DataPointByTopic = {};
    for(let i = 0; i < data.length; i++){
        const currentDataPoint = data[i];
        const currentTopic = currentDataPoint.topic;

        if(!Array.isArray(dataByTopic[currentTopic])){
            dataByTopic[currentTopic] = [];
        }
        dataByTopic[currentTopic].push(currentDataPoint);
    }
    return dataByTopic
}
