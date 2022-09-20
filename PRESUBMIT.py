# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/. */

USE_PYTHON3 = True
PRESUBMIT_VERSION = '2.0.0'


def CheckChangeLintsClean(input_api, output_api):
    return input_api.canned_checks.CheckChangeLintsClean(input_api,
                                                         output_api,
                                                         lint_filters=[])


def _CheckPylint(input_api, output_api):
    return input_api.canned_checks.RunPylint(
        input_api,
        output_api,
        pylintrc=input_api.os_path.join(input_api.PresubmitLocalPath(),
                                        '.pylintrc'),
        version='2.7')


def CheckLicense(input_api, output_api):
    """Verifies the Brave license header."""

    current_year = int(input_api.time.strftime('%Y'))
    allowed_years = (str(s) for s in reversed(range(2015, current_year + 1)))
    years_re = '(' + '|'.join(allowed_years) + ')'

    # A file that lacks this line necessarily lacks a compatible license.
    # Checking for this line lets us avoid the cost of a complex regex across a
    # possibly large file. This has been seen to save 50+ seconds on a single
    # file.
    key_line = 'This Source Code Form is subject to the terms of the Mozilla Public'
    license_re = (
        r'.*? Copyright (\(c\) )?%(year)s The Brave Authors\. All rights reserved\.?\r?\n'
        r'.*? %(key_line)s\r?\n?'
        r'.*? License, v\. 2\.0\. If a copy of the MPL was not distributed with this?\r?\n?.*?file,?\r?\n?'
        r'.*? (Y|y)ou can obtain one at https?://mozilla.org/MPL/2\.0/\.(?: \*/)?\r?\n'
    ) % {
        'year': years_re,
        'key_line': key_line,
    }

    license_re = input_api.re.compile(license_re, input_api.re.MULTILINE)
    bad_files = []
    bad_new_files = False
    files_to_check = input_api.DEFAULT_FILES_TO_CHECK + (r'.+\.gni?$', )
    sources = lambda affected_file: input_api.FilterSourceFile(
        affected_file,
        files_to_check=files_to_check,
        files_to_skip=input_api.DEFAULT_FILES_TO_SKIP)
    for f in input_api.AffectedSourceFiles(sources):
        contents = input_api.ReadFile(f, 'r')
        if not contents:
            continue
        # Search for key_line first to avoid fruitless and expensive regex searches.
        if key_line not in contents:
            bad_files.append(f.LocalPath())
            if f.Action() == 'A':
                bad_new_files = True
        elif not license_re.search(contents):
            bad_files.append(f.LocalPath())
            if f.Action() == 'A':
                bad_new_files = True
    if bad_new_files:
        return [
            output_api.PresubmitError(
                'License must match:\n%s\n' % license_re.pattern +
                'Found a bad license header in these files, some of which are new:',
                items=bad_files)
        ]
    if bad_files:
        return [
            output_api.PresubmitPromptWarning(
                'License must match:\n%s\n' % license_re.pattern +
                'Found a bad license header in these files:',
                items=bad_files)
        ]
    return []
