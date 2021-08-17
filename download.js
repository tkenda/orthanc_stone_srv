const https = require('https');
const unzipper = require('unzipper');
const fs = require('fs-extra');

const URL = 'https://lsb.orthanc-server.com/stone-webviewer/mainline/wasm-binaries.zip';
const SRC_DIR = './temp/wasm-binaries/StoneWebViewer';
const DST_DIR = './front';
const TMP_DIR = './temp';
const FILE_DIR = 'front.zip';

var download = function (url, dest, cb) {
    var file = fs.createWriteStream(dest);
    https.get(url, function (response) {
        response.pipe(file);
        file.on('finish', function () {
            file.close(cb);
        });
    }).on('error', function (err) {
        fs.unlink(dest);
        if (cb) cb(err.message);
    });
};

fs.emptyDir(DST_DIR, err => {
    if (err) return console.error(err);
    console.log('Front folder cleaned.');
    console.log('Downloading..');

    download(URL, FILE_DIR, function (err) {
        if (err) return console.error(err);
        console.log('Zip file downloaded.');

        fs.createReadStream(FILE_DIR)
            .pipe(unzipper.Extract({ path: TMP_DIR }))
            .on('close', function (err) {
                if (err) return console.error(err);
                console.log('File unzipped.');

                fs.copySync(SRC_DIR, DST_DIR, { overwrite: true });
                fs.emptyDirSync(TMP_DIR);
                fs.rmdirSync(TMP_DIR, { recursive: true });
                fs.unlinkSync(FILE_DIR);
                console.log('Ready!');
            });
    });
});