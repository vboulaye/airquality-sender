

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
