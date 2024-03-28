from flask import (
    Blueprint, render_template
)
from werkzeug.exceptions import abort

settings_bp = Blueprint('settings', __name__,template_folder='templates')

@settings_bp.route('/settings')
def settings_home():
    return render_template("settings.html")