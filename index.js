var http = require('http');
var httpProxy = require('http-proxy');
var static = require('node-static');
const path = require('path');

var front = new(static.Server)(path.join(__dirname, 'front'));
var proxy = httpProxy.createProxyServer({});

proxy.on('proxyReq', function(proxyReq, _req, _res, _options) {
    proxyReq.setHeader('X-WADO-Client', 'orthanc');
});

var server = http.createServer(function(req, res) {
    if (req.url.split('/')[1] == 'dicom-web') {
        proxy.web(req, res, { target: 'http://127.0.0.1:8080/api/v1/' });
    } else {
        front.serve(req, res);
    }
});

server.listen(3000);