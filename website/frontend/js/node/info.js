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
        type: 'bar',
        data: {
            labels: labels,
            datasets: [{
                label: '#added websites by days',
                data: values,
                backgroundColor: [
                    'rgba(255, 99, 132, 0.2)',
                    'rgba(54, 162, 235, 0.2)',
                    'rgba(255, 206, 86, 0.2)',
                    'rgba(75, 192, 192, 0.2)',
                    'rgba(153, 102, 255, 0.2)',
                    'rgba(255, 159, 64, 0.2)'
                ],
                borderColor: [
                    'rgba(255, 99, 132, 1)',
                    'rgba(54, 162, 235, 1)',
                    'rgba(255, 206, 86, 1)',
                    'rgba(75, 192, 192, 1)',
                    'rgba(153, 102, 255, 1)',
                    'rgba(255, 159, 64, 1)'
                ],
                borderWidth: 1
            }]
        },
        options: {
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