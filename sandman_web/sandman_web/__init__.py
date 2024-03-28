import os

from flask import Flask,request,redirect

def create_app(test_config = None):
    """Creates and configures the app.

    test_config - Which testing configuration to use, if any.
    """

    app = Flask(__name__, instance_relative_config = True)
    app.config.from_mapping(
        SECRET_KEY='dev',
    )

    if test_config is None:
        # Load the instance configwhen not testing, if it exists.
        app.config.from_pyfile('config.py', silent = True)
    else:
        # Otherwise use the test config.
        app.config.from_mapping(test_config)

    # Make sure the instance folder exists.
    try:
        os.makedirs(app.instance_path)
    except OSError:
        pass

    # Sandman Reports Home Page
    @app.route('/')
    def home():
        return reports.index()

    # A path to redirect to the rhasspy admin home page.
    @app.route('/rhasspy')
    def rhasspy():
        server_ip = request.host.split(':')[0]
        return redirect("http://" + server_ip + ':12101')

    # Register blueprints 
    from .reports import reports
    app.register_blueprint(reports.blueprint)

    from .settings import settings
    app.register_blueprint(settings.settings_bp)

    from .status import status
    app.register_blueprint(status.status_bp)

    # Create global status variable
    @app.context_processor
    def status_processor():
        if status.check_sandman_health() == 0 and status.check_rhasspy_health() == 0:
            health_issue = False
        else:
            health_issue = True
        return dict(health_issue = health_issue)

    return app