import { useRef } from "react"

interface DatePickerProps {
    timestamp: number;
    onOk: (timestamp: number) => void;
    close?: () => void;
}

const SingleDateSelector = ({
    onOk,
    timestamp,
    close,
}: DatePickerProps) => {
    const timestampRef = useRef<HTMLInputElement>(null);

    return (
        <div>
            <label htmlFor="timestamp">Timestamp: </label>
            <input
                ref={timestampRef}
                className="date-selection-input"
                type="datetime-local"
                name="start"
                defaultValue={new Date(timestamp*1000).toISOString().slice(0, 16)}
            />
            <button
                onClick={() => {
                    if(!timestampRef.current){
                        return;
                    }
                    onOk(Math.floor(new Date(timestampRef.current.value).valueOf()/1000))
                    if(close){
                        close();
                    }
                }}
            >Ok</button>
        </div>
    )
}

export default SingleDateSelector