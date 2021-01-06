# SPDX-License-Identifier: MIT
# Copyright (c) 2020 The Authors.
#
# Authors: Bin Liang <@liangbin>
#
# Summary: Configuration for Zeta-manager NBI API
#
import os

POSTGRES_USER = os.environ.get('POSTGRES_USER')
POSTGRES_PASSWORD = os.environ.get('POSTGRES_PASSWORD')
POSTGRES_SERVICE_HOST = os.environ.get('POSTGRES_SERVICE_HOST')
POSTGRES_SERVICE_PORT = os.environ.get('POSTGRES_SERVICE_PORT')

DATABASE_URL = f'postgres://{POSTGRES_USER}:{POSTGRES_PASSWORD}@postgres:{POSTGRES_SERVICE_PORT}/zeta'


class BaseConfig:
    """Base configuration"""
    STAGE = 'production'
    DEBUG = False
    TESTING = False
    SECRET_KEY = "98d0s809SD990AS)(dS&A&*d78(*&ASD08A"
    SQLALCHEMY_ECHO = False
    SQLALCHEMY_TRACK_MODIFICATIONS = False
    SQLALCHEMY_DATABASE_URI = DATABASE_URL


class DevelopmentConfig(BaseConfig):
    """Development configuration"""
    STAGE = "development"
    DEBUG = True


class ProductionConfig(BaseConfig):
    """Production configuration"""
    SECRET_KEY = os.environ.get('SECRET_KEY')
