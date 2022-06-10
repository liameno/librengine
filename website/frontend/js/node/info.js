function http_get(the_url) {
    let xmlHttpReq = new XMLHttpRequest();
    xmlHttpReq.open("GET", the_url, false);
    xmlHttpReq.send(null);
    return xmlHttpReq.responseText;
}

function chart() {
    let data = JSON.parse(http_get("/api/node/info/chart"))["data"];
    let keys = Object.keys(data);

    let labels = [];
    let values = [];

    for (let i = 0; i < keys.length; i++) {
        let key = keys[i];
        let value = data[key];

        labels.push(key);
        values.push(value);
    }

    const ctx = document.getElementById('chart').getContext('2d');
    const chart = new Chart(ctx, {
        type: 'line',
        data: {
            labels: labels,
            datasets: [{
                label: '#added websites by days',
                data: values,
                backgroundColor: [
                    '#70c17c',
                ],
                borderColor: [
                    '#7DE38D',
                ],
                borderWidth: 1,
                //fill: 'start'
            }]
        },
        options: {
            elements: {
              line: {
                  //tension: 0.2
              }
            },
            scales: {
                y: {
                    beginAtZero: true,
                    grid: {
                        color: 'rgba(200, 200, 200, 0.1)'
                    }
                },
                x: {
                    grid: {
                        color: 'rgba(200, 200, 200, 0.1)'
                    }
                }
            }
        }
    });
}