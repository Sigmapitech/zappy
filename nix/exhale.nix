{
  buildPythonPackage,
  fetchFromGitHub,
  setuptools,
  breathe,
  sphinx,
  beautifulsoup4,
  lxml,
  six,
  lib,
}:
buildPythonPackage rec {
  pname = "exhale";
  version = "0.3.7";
  pyproject = true;

  src = fetchFromGitHub {
    owner = "svenevs";
    repo = "exhale";
    tag = "v${version}";
    hash = "sha256-I7Q2vKLT/h35xX87FugyvxSTESnO3+LFLUX9kZOPI0I=";
  };

  build-system = [setuptools];

  dependencies = [
    breathe
    sphinx
    beautifulsoup4
    lxml
    six
  ];

  pythonImportsCheck = ["exhale"];

  doCheck = false; # no great

  meta = {
    description = "Automatic C++ library api documentation generation";
    license = lib.licenses.bsd3;
  };
}
