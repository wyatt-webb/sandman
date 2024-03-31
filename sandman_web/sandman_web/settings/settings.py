from flask import (
    Blueprint, render_template, send_file
)
from werkzeug.exceptions import abort
import datetime

# File path constant
CONFIG_FILE_PATH = "/usr/local/etc/sandman/sandman.conf"

settings_bp = Blueprint('settings', __name__, url_prefix='/settings', template_folder='templates', static_folder='static')

@settings_bp.route('/')
def settings_home():
    return render_template("settings.html")

@settings_bp.route('/downloadconfig')
def download_config():
    '''
    Download the Sandman config file to the client's device.
    '''
    download_filename = "sandman_"
    # Get the date and time
    timestamp = datetime.datetime.now()
    # Create time string of local date-local time
    timestamp_string = timestamp.strftime("%x" + "-" + "%X")
    # Remove colons so filesystems are happy
    timestamp_string = timestamp_string.replace(":", "_")
    download_filename += timestamp_string
    # Download the config file to the client machine
    return send_file(CONFIG_FILE_PATH, as_attachment=True, download_name= download_filename + ".conf")