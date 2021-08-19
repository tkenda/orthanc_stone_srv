let axios = require('axios');

var readStudyIuid = function (req, cb) {
    var iuid = '';

    req.on('data', function (data) {
        iuid += data;

        // Too much POST data, kill the connection!
        // 1e6 === 1 * Math.pow(10, 6) === 1 * 1000000 ~~~ 1MB
        if (iuid.length > 1e6) {
            req.connection.destroy();
            cb(null);
        }
    });

    req.on('end', function () {
        cb(iuid);
    });
}

function response(res, obj) {
    res.setHeader('Content-Type', 'application/json; charset=utf-8');
    res.end(JSON.stringify(obj));
}

module.exports = function(server, req, res) {
    readStudyIuid(req, function(iuid) {
        if (iuid) {
            axios.post(server.proxy + 'dicom/find/studies', {
                studyIuid: [iuid]
            }).then(proteus => {
                if (proteus.data.status === 'done') {
                    let ret = new Array();
                    for (let i=0;i<proteus.data.studies.length;i++) {
                        ret.push({
                            "ID": proteus.data.studies[i].studyDbId,
                            "Path": "/studies/"+proteus.data.studies[i].studyDbId,
                            "Type": "Study"
                        });
                    }
                    response(res, ret);
                } else {
                    console.log('Invalid response!');
                    response(res, []);
                }
            }).catch(error => {
                console.log(error);
                response(res, []);
            });
        } else {
            response(res, []);
        }
    });
};