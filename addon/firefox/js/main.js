function compute_time() {
    let now = new Date;
    return Date.UTC(now.getFullYear(), now.getMonth(), now.getDate(), now.getHours(), now.getMinutes(), now.getSeconds(), now.getMilliseconds());
}
function compute_website_json(title, url, host, desc) {
    let json = {
        "title": title,
        "url": url,
        "host": host,
        "desc": desc.toString(),
        "date": compute_time(),
        "rating": 100
    };

    return JSON.stringify(json);
}
function compute_search_website_json(field, phrase) {
    let now = compute_time();
    let json = {
        "query": {"bool": {"must": [
                    {"match": {"url"/*field //TODO: fix*/: phrase}},
                    {"range": {"date": {"gte": now - 86400, "lte": now}}}
                ]}}
    };

    return JSON.stringify(json);
}
function post_request(url, data) {
    return fetch(url, {
        method: 'POST',
        body: data,
        headers: new Headers({
            'Content-Type': 'application/json'
        }),
    }).then(response => response.json())
}
async function post(url, data) {
    let response = await fetch(url, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: data
    });

    return await response.json();
}

let button = document.getElementById("add");

function add_click() {
    button.disabled = true;

    browser.tabs.query({active: true, currentWindow: true}).then((tabs) => {
        let tab = tabs[0];
        let url = new URL(tab.url);

        const executing = browser.tabs.executeScript({
            file: "/js/tab.js"
        });
        executing.then(function (result) {
            let data = compute_search_website_json("url", url);
            let data2 = compute_website_json(tab.title, tab.url, url.hostname, result);

            post_request("http://localhost:9200/website/_search", data).then(data => {
                let hits_count_added = data["hits"]["total"]["value"];
                document.getElementById("status").innerText = hits_count_added;

                if (hits_count_added > 0) {
                    document.getElementById("status").innerText = "Already added";
                    return 1;
                } else {
                    post("http://localhost:9200/website/_doc", data2) //TODO: from config
                    document.getElementById("status").innerText = "Added [" + url + "]";
                }
            }); //TODO: from config
        }, function (error) {
            document.getElementById("status").innerText = error;
        });
    }, console.error);
}

const executing = browser.tabs.executeScript({
    code: "var meta_tags; var content;"
});

button.addEventListener("click", add_click);
//TODO: fix double click (set timeout)
