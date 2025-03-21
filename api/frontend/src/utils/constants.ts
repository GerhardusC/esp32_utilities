export const tzOffsetMillis = new Date().getTimezoneOffset()*60*1000;

export const units = new Map([
    ["temperature", "°C"],
    ["humidity", "%"],
]);

export const topicColors = new Map([
    ["temperature", "#e6b41e"],
    ["humidity", "#4786ff"],
])