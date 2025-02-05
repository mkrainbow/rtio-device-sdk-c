
const RTIO_SERVICE = "http://demo.mkrainbow.local:17917";   // Replace with your RTIO service address.
const DEVICE_ID = "cfa09baa-4913-4ad7-a936-0e26f9671b06";   // Replace with your device ID.
const REMOTE_URL = RTIO_SERVICE + "/" + DEVICE_ID;


let controller;

function logShow(log) {
    var t = document.getElementById("log_area");
    t.value = t.value + log + '\n';
    t.scrollTop = t.scrollHeight;
}
function gpioStatusShow(level) {
    var t = document.getElementById("gpio_status");
    if (level) {
        t.textContent = "The GPIO Status " + "↗️ 1";
    } else {
        t.textContent = "The GPIO Status " + "↘️ 0";
    }
}

async function* makeTextFileLineIterator() {

    controller = new AbortController();
    const utf8Decoder = new TextDecoder("utf-8");

    try {
        const response = await fetch(REMOTE_URL, {
            method: 'POST',
            body: '{"method":"obget", "uri":"/gpio","id":12668,"data":""}',
            signal: controller.signal,
        })
        const reader = response.body.getReader();
        let { value: chunk, done: readerDone } = await reader.read();
        chunk = chunk ? utf8Decoder.decode(chunk) : "";

        const newline = /\r?\n/gm;
        let startIndex = 0;
        let result;

        while (true) {
            const result = newline.exec(chunk);
            if (!result) {
                if (readerDone) break;
                const remainder = chunk.substr(startIndex);
                ({ value: chunk, done: readerDone } = await reader.read());
                chunk = remainder + (chunk ? utf8Decoder.decode(chunk) : "");
                startIndex = newline.lastIndex = 0;
                continue;
            }
            yield chunk.substring(startIndex, result.index);
            startIndex = newline.lastIndex;
        }

        if (startIndex < chunk.length) {
            // Last line didn't end in a newline char
            yield chunk.substr(startIndex);
        }
    } catch (err) {
        console.log("fetch cancelled:", err);
        logShow('Observing GPIO stoped (' + err.message + ').');
    }
}

async function observe() {

    logShow('\nObserving GPIO status.');
    logShow("URL: " + REMOTE_URL);
    logShow('BODY: \'{"method":"obget", "uri":"/gpio","id":12668,"data":""}\'');

    for await (const line of makeTextFileLineIterator()) {
        console.log(line);
        const jsonObj = JSON.parse(line);

        logShow("RESP: " + JSON.stringify(jsonObj));
        if (jsonObj.hasOwnProperty("data")) {
            const decodedText = atob(jsonObj["data"]);
            logShow("=> BASE64 decoded DATA: " + decodedText);

            const matched = decodedText.match(/GPIO=\[(\d+)\]/);
            if (matched) {
                gpioStatusShow(Number(matched[1]))
            }
        }
    }
}

var bt_on = document.getElementById("bt_on");
bt_on.style.color = 'green';
bt_on.onclick = function () {
    if (bt_on.style.color == 'gray') {
        console.log("click invalid");
        logShow("click invalid");
        return;
    }
    bt_on.style.color = 'gray';
    console.log("click START...");
    observe()
}

var bt_off = document.getElementById("bt_off");
bt_off.style.color = 'green';
bt_off.onclick = function () {
    if (bt_off.style.color == 'gray') {
        console.log("click invalid");
        logShow("click invalid");
        return;
    }
    bt_off.style.color = 'gray';
    console.log("click STOP...");
    if (controller) {
        controller.abort("click STOP");
        console.log("observe fetch abort");
        delete controller
    }
    bt_off.style.color = 'green';
    bt_on.style.color = 'green';
}
