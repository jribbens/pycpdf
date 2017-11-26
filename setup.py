"""setup.py for pycpdf"""

import os
import re
from setuptools import Extension, setup


def get_version():
    with open(os.path.join(os.path.dirname(__file__), 'pycpdfmodule.c'),
              'r') as source:
        for line in source:
            match = re.match(r'#define PYCPDF_VERSION "([0-9.]+)"', line)
            if match:
                return match.group(1)
        else:
            raise Exception("Couldn't find PYCPDF_VERSION in pycpdfmodule.c")


setup(
    name='pycpdf',
    version=get_version(),
    description='Extract content and metadata from PDF files efficiently',
    author='Jon Ribbens',
    url='https://github.com/jribbens/pycpdf',
    license='MIT',
    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: MIT License',
        'Programming Language :: Python :: 2',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3.4',
        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
    ],
    keywords='pdf pypdf',
    ext_modules=[
        Extension('pycpdf', ['pycpdfmodule.c']),
    ],
    test_suite='tests',
)
