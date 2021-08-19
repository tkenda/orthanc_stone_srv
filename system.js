module.exports = function(server, _req, res) {
    let isHttpServerSecure;

    if (typeof server.isHttpServerSecure !== 'undefined') {
        isHttpServerSecure = server.isHttpServerSecure;
    } else {
        isHttpServerSecure = false;
    }

    res.setHeader('Content-Type', 'application/json');
    res.end(JSON.stringify({
        "ApiVersion": 13,
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
};