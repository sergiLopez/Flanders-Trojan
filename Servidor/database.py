import os
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker



if False:
    SQLALCHEMY_DATABASE_URL = os.getenv("DATABASE_URL")
else:
    SQLALCHEMY_DATABASE_URL = "sqlite:///./data.db"


engine = create_engine(
    SQLALCHEMY_DATABASE_URL, connect_args={"check_same_thread": False})

SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)

Base = declarative_base()