import sys
import os
from setuptools import setup, find_packages
from distutils.core import setup, Extension

curPath = os.path.abspath(os.path.dirname(__file__))
rootPath = os.path.split(curPath)[0]
sys.path.append(rootPath)


setup (name = 'yael',
       #version = '',
       #url = 'https://github.com/aaalgo/kgraph',
       #author = 'Wei Dong',
       #author_email = 'wdong@wdong.org',
       #license = 'BSD',
       #description = 'Yael Library',
       #packages = find_packages(),
       #include_package_data = True,
       #install_requires = []
       )
