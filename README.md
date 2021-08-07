# Non-official fork from Orthanc's [Stone Web Viewer](https://www.orthanc-server.com/static.php?page=stone-web-viewer)

The front end is already built. The NodeJS static web server acts also as a proxy to the DICOM WADO server.

- Install NodeJS
```
curl -fsSL https://deb.nodesource.com/setup_lts.x | sudo -E bash -
sudo apt-get install -y nodejs
```

- Configure correctly server.json.
- Install dependencies and run server.

```
npm install
npm start
```

### You could build the latest version (only for Ubuntu)

Install Docker following the official page https://docs.docker.com/engine/install/ubuntu/

```
sudo sh build.sh
```
