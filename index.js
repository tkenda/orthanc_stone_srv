var http = require('http');
var httpProxy = require('http-proxy');
var static = require('node-static');
const path = require('path');
const fs = require('fs');

const server = JSON.parse(fs.readFileSync('server.json'));

var front = new(static.Server)(path.join(__dirname, 'front'));
var proxy = httpProxy.createProxyServer({});

proxy.on('error', function (error, _req, res) {
    if (!res.headersSent) {
        res.writeHead(500, { 'content-type': 'application/json' });
    }

    res.end(JSON.stringify({ 
        error: 'proxy_error', 
        reason: error.message 
    }));
});

if (typeof server.header !== 'undefined') {
    if (typeof server.header.name !== 'undefined' && typeof server.header.value !== 'undefined') {
        proxy.on('proxyReq', function(proxyReq, _req, _res, _options) {
            proxyReq.setHeader(server.header.name, server.header.value);
        });
    } else {
        console.log("Invalid header name or value in server.json file.");
        return;
    }
}

if (typeof server.proxy === 'undefined') {
    console.log("Missing proxy URL in server.json file.");
    return;
} else {
    console.log("Proxy URL: "+server.proxy);
}

const port = (typeof server.port === 'undefined') ? 3000 : server.port;

http.createServer(function(req, res) {
    if (req.url.split('/')[1] == 'dicom-web') {
        proxy.web(req, res, { target: server.proxy });
    } else {
        front.serve(req, res);
    }
}).listen(port);

console.log("Listening on port "+port+".");