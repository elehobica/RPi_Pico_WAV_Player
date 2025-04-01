const core = require('@actions/core');
const fs = require('fs');

try {
  const targetFile = core.getInput('target', {required: true});
  let text = fs.readFileSync(targetFile, 'utf-8');
  let version_str = core.getInput('version_str', {required: true});
  fs.writeFileSync(targetFile, text.replace(/("CFG_VERSION",\s+)"\d\.\d\.\d"/, `$1"${core.getInput('version_str')}"`));
} catch (error) {
    core.setFailed(error.message);
}