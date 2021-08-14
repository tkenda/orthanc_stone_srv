var http = require('http');
var httpProxy = require('http-proxy');
var static = require('node-static');
const path = require('path');
const fs = require('fs');
const { type } = require('os');

const server = JSON.parse(fs.readFileSync('server.json'));

var front = new (static.Server)(path.join(__dirname, 'front'));
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
        proxy.on('proxyReq', function (proxyReq, _req, _res, _options) {
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
    console.log("Proxy URL: " + server.proxy);
}

const port = (typeof server.port === 'undefined') ? 3000 : server.port;

http.createServer(function (req, res) {
    let chunk = req.url.split('/')[1];
    if (chunk == 'dicom-web') {
        proxy.web(req, res, { target: server.proxy });
    } else if (chunk == 'system') {
        let isHttpServerSecure;
        if (typeof server.isHttpServerSecure !== 'undefined') {
            isHttpServerSecure = server.isHttpServerSecure;
        } else {
            isHttpServerSecure = false;
        }
        res.setHeader('Content-Type', 'application/json');
        res.end(JSON.stringify({
            "ApiVersion": 14,
            "CheckRevisions": false,
            "DatabaseBackendPlugin": null,
            "DatabaseVersion": 0,
            "DicomAet": "STONE",
            "DicomPort": 4242,
            "HttpPort": 8042,
            "IsHttpServerSecure": isHttpServerSecure,
            "Name": "MyStone",
            "PluginsEnabled": false,
            "StorageAreaPlugin": null,
            "Version": "1.9.5"
        }));
    } else {
        front.serve(req, res);
    }
}).listen(port);

console.log("Listening on port " + port + ".");