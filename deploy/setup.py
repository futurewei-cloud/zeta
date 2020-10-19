from setuptools import setup, find_packages

setup(
    name='zeta',
    version='0.1.0',
    packages=find_packages(include=['zeta', 'zeta.*']),
    install_requires=[
        'PyYAML',
        'kopf',
        'netaddr',
        'ipaddress',
        'pyroute2',
        'kubernetes==11.0.0',
        'luigi==2.8.12',
        'grpcio',
        'protobuf',
        'fs'
    ]
)
