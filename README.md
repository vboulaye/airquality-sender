
# air quality sender [![Build Status](https://travis-ci.com/vboulaye/airquality-sender.svg?branch=master)](https://travis-ci.com/vboulaye/airquality-sender) ![publish jupyter notebook](https://github.com/vboulaye/airquality-sender/workflows/run%20jupyter%20notebook/badge.svg)


Reads PM2.5 index (as well as PM1 and PM10)  using a PMS5003 sensor.

Sends the data to a [webcom database](https://datasync.orange.com/)

Results are graphed using [jupyter notebook](https://jupyter.org/) and published using [github actions](https://help.github.com/en/actions) to [github pages](https://vboulaye.github.io/airquality-sender/)



## install

Install Jupyter and its dependencies in a virtual env:

```
python3 -m venv  venv

source venv/bin/activate

pip3 install -r requirements.txt

```

setup the configuration from the example `airquality-config.json.example`
you'll need a database and its auth key from [webcom](https://datasync.orange.com/)


start jupyter:

```
source venv/bin/activate
jupyter notebook
```

