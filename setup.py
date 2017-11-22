"""setup.py for pycpdf"""

from setuptools import Extension, setup

setup(
    name='pycpdf',
    version='1.0.0',
    description='Extract content and metadata from PDF files efficiently',
    author='Jon Ribbens',
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
