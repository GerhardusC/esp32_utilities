import { useRef, useState } from "react";

type DateRangePickerProps = {
    startStop: [number, number];
    onOk: (startStop: [number, number]) => void;
    close?: () => void;
}

const DateRangePicker = ({
    close,
    onOk,
    startStop,
}: DateRangePickerProps) => {
    const [invalidState, setInvalidState] = useState<"none" | "biggerStartDate">("none");
    const startDateRef = useRef<HTMLInputElement>(null);
    const stopDateRef = useRef<HTMLInputElement>(null);

    return (
        <>
            <div>
                <label htmlFor="start">Start: </label>
                <input
                    ref={startDateRef}
                    className="date-selection-input"
                    type="datetime-local"
                    name="start"
                    defaultValue={new Date(startStop[0]).toISOString().slice(0, 16)}
                    onChange={(e) => {
                        if(!stopDateRef.current){
                            return;
                        }
                        if(new Date(e.target.value) > new Date(stopDateRef.current.value)){
                            setInvalidState("biggerStartDate");
                        } else {
                            setInvalidState("none");
                        }
                    }}
                />
            </div>
            <div>
                <label htmlFor="stop">Stop: </label>
                <input
                    ref={stopDateRef}
                    className="date-selection-input"
                    defaultValue={new Date(startStop[1]).toISOString().slice(0, 16)}
                    type="datetime-local"
                    name="stop"
                    onChange={(e) => {
                        if(!startDateRef.current){
                            return;
                        }
                        if(new Date(e.target.value) < new Date(startDateRef.current.value)){
                            setInvalidState("biggerStartDate");
                        } else {
                            setInvalidState("none");
                        }
                    }}
                />
            </div>
            <button
                disabled={invalidState !== "none"}
                onClick={() => {
                    if(!startDateRef.current || !stopDateRef.current){
                        return;
                    }
                    if(invalidState !== "none"){
                        return;
                    }
                    onOk([Math.floor(new Date(startDateRef.current.value).valueOf()/1000), Math.floor(new Date(stopDateRef.current.value).valueOf()/1000)])
                    if(close){
                        close();
                    }
                }}
            >Ok</button>
        </>
    )
}

export default DateRangePicker