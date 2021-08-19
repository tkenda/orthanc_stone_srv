module.exports = function(server, req, res) {
    try {
        res.writeHead(301, {
            Location: server.proxy + 'dicom/zip/' + req.url.split('/')[2]
        });
        res.end();
    } catch(err) {
        console.log(err);
        res.end();
    }
};