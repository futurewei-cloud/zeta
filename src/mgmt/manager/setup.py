from setuptools import setup, find_packages

setup(
    name='manager',
    version='0.1.0',
    description='Zeta manager package',
    author='Bin Liang',
    packages=find_packages(),
    install_requires=[
        'PyYAML',
        'Flask>=1.1.1',
        'Flask-Cors>=3.0.8',
        'flask-migrate>=2.5.2',
        'Flask-SQLAlchemy>=2.4.1',
        'gunicorn>=20.0.4',
        'psycopg2-binary==2.8.4',
        'kubernetes==11.0.0'
    ],
    setup_requires=['pytest-runner', 'flake8'],
    tests_require=['pytest'],
    entry_points={
        'console_scripts': ['manager=manager.manager:main']
    },
    package_data={'exampleproject': ['data/schema.json']}
)
