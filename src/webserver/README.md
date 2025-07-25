# webserver

Run a webserver to switch between mock USB MSC and FTP modes.

## :pencil: Usage

Get the current mode

```shell
curl -X GET http://<ip-address>/
```

Switch to FTP mode

```shell
curl -X POST http://<ip-address>/ftp
```

Switch to MSC mode

```shell
curl -X POST http://<ip-address>/msc
```

## :link: Refereneces
