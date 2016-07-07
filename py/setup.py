#! /usr/bin/env python3

try:
    from setuptools import setup
except:
    from distutils.core import setup

setup(
    name = "minilocker",
    version = "0.1",
    packages = ["minilocker"],
    scripts = ["bin/minilocker-dump"],
    description = "MiniLocker interface library + binaries",
    author = "Josef Gajdusek",
    author_email = "atx@atx.name",
    url = "https://github.com/atalax/minilocker",
    license = "MIT",
    classifiers = [
        "Programming Language :: Python",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        ]
    )
