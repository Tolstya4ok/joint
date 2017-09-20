echo "Sending coverage info"

lcov --directory . --capture --output-file coverage.info &&
lcov --remove coverage.info '/usr/*' --output-file coverage.info &&
lcov --list coverage.info && coveralls-lcov --repo-token ${COVERALLS_TOKEN} coverage.info ||
exit 1