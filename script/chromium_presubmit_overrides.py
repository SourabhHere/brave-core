# Copyright (c) 2022 The Brave Authors. All rights reserved.
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/. */

# This file is executed (not imported) in the context of src/PRESUBMIT.py, it
# uses existing functions from a global scope to change them. This allows us to
# alter root Chromium presubmit checks without introducing too much conflict.

import copy
import inspect
import re
import sys

import lib.chromium_presubmit_utils as chromium_presubmit_utils
import override_utils
import import_inline

# pylint: disable=line-too-long
# pylint: disable=protected-access

_BRAVE_DEFAULT_FILES_TO_SKIP = (r'win_build_output[\\/].*', )


def _load_json5(file_path):
    try:
        json5_path = import_inline.join_src_dir('third_party', 'pyjson5',
                                                'src')
        sys.path.append(json5_path)
        # pylint: disable=import-outside-toplevel,import-error
        import json5
        return json5.load(open(file_path))
    finally:
        # Restore sys.path to what it was before.
        sys.path.remove(json5_path)


def _apply_blocklist_to_AffectedFiles(input_api):
    def get_check_names(frame):
        check_names = set()
        while frame:
            co_name = frame.f_code.co_name
            if co_name.startswith('Check') or co_name.startswith('_Check'):
                check_names.add(co_name)
            if co_name == '_run_check_function':
                break
            frame = frame.f_back
        return check_names

    check_files_to_skip = _load_json5(
        import_inline.join_src_dir('brave',
                                   'chromium_presubmit_blocklist.json5'))

    def get_files_to_skip(check_names):
        files_to_skip = []
        for check_name in check_names:
            files_to_skip.extend(check_files_to_skip.get(check_name, []))
        return files_to_skip

    # pylint: disable=unused-variable
    @override_utils.override_method(input_api.change)
    def AffectedFiles(_self, original_method, *args, **kwargs):
        files_to_skip = get_files_to_skip(
            get_check_names(inspect.currentframe().f_back))
        affected_files = input_api.change._affected_files
        if files_to_skip:

            def file_filter(affected_file):
                local_path = affected_file.LocalPath().replace('\\', '/')
                for file_to_skip in files_to_skip:
                    if re.match(file_to_skip, local_path):
                        return False
                return True

            affected_files = list(filter(file_filter, affected_files))

        with override_utils.override_scope_variable(input_api.change,
                                                    '_affected_files',
                                                    affected_files):
            return original_method(*args, **kwargs)


# Modify depot_tools-bundled checks (Chromium calls them canned checks).
# These modification will stay active across all PRESUBMIT.py files, i.e.
# src/PRESUBMIT.py, src/brave/PRESUBMIT.py.
def _modify_canned_checks(canned_checks):
    # pylint: disable=unused-variable

    # Disable upstream-specific license check.
    @chromium_presubmit_utils.override_check(canned_checks)
    def CheckLicense(*_, **__):
        return []

    # We don't use OWNERS files.
    @chromium_presubmit_utils.override_check(canned_checks)
    def CheckOwnersFormat(*_, **__):
        return []

    # We don't use OWNERS files.
    @chromium_presubmit_utils.override_check(canned_checks)
    def CheckOwners(*_, **__):
        return []

    # We don't use AUTHORS file.
    @chromium_presubmit_utils.override_check(canned_checks)
    def CheckAuthorizedAuthor(*_, **__):
        return []

    # We don't upload change to Chromium git.
    @chromium_presubmit_utils.override_check(canned_checks)
    def CheckChangeWasUploaded(*_, **__):
        return []

    # We don't upload change to Chromium git.
    @chromium_presubmit_utils.override_check(canned_checks)
    def CheckChangeHasBugField(*_, **__):
        return []

    # We don't upload change to Chromium git.
    @chromium_presubmit_utils.override_check(canned_checks)
    def CheckTreeIsOpen(*_, **__):
        return []

    # Changes from upstream:
    # 1. Generate PresubmitError instead of Warning on format issue.
    # 2. Replace suggested command from upstream-specific to 'npm run format'.
    @chromium_presubmit_utils.override_check(canned_checks)
    def CheckPatchFormatted(original_check, input_api, output_api, *args,
                            **kwargs):
        kwargs = {
            **kwargs,
            'bypass_warnings': False,
            'check_python': True,
            'result_factory': output_api.PresubmitError,
        }
        result = original_check(input_api, output_api, *args, **kwargs)
        # If presubmit generates "Please run git cl format --js" message, we
        # should replace the command with "npm run format -- --js". The order of
        # these replacements ensure we do this properly.
        replacements = [
            (' format --', ' format -- --'),
            ('git cl format', 'npm run format'),
            ('gn format', 'npm run format'),
            ('rust-fmt', 'rust'),
            ('swift-format', 'swift'),
        ]
        for item in result:
            for replacement in replacements:
                item._message = item._message.replace(replacement[0],
                                                      replacement[1])
        return result

    # Changes from upstream:
    # 1. Force PresubmitErrors instead of Warnings.
    @chromium_presubmit_utils.override_check(canned_checks)
    def CheckChangeLintsClean(original_check, input_api, output_api, **kwargs):
        with chromium_presubmit_utils.force_presubmit_error(output_api):
            return original_check(input_api, output_api, **kwargs)

    # Changes from upstream:
    # 1. Run lint only on *changed* files instead of getting *all* files from
    #    the directory. Upstream does it to catch breakages in unmodified files,
    #    but it's very resource intensive, moreover for our setup it covers all
    #    files from vendor and other directories which we should ignore.
    # 2. Force PresubmitErrors instead of Warnings.
    @chromium_presubmit_utils.override_check(canned_checks)
    def GetPylint(original_check, input_api, output_api, **kwargs):
        def _FetchAllFiles(_, input_api, files_to_check, files_to_skip):
            src_filter = lambda f: input_api.FilterSourceFile(
                f, files_to_check=files_to_check, files_to_skip=files_to_skip)
            return [
                f.LocalPath()
                for f in input_api.AffectedSourceFiles(src_filter)
            ]

        with override_utils.override_scope_function(input_api.canned_checks,
                                                    _FetchAllFiles):
            with chromium_presubmit_utils.force_presubmit_error(output_api):
                return original_check(input_api, output_api, **kwargs)


# Override the first ever check defined in PRESUBMIT.py to make changes to
# input_api before any real check is run.
@chromium_presubmit_utils.override_check(
    globals(), chromium_presubmit_utils.get_first_check_name(globals()))
def OverriddenFirstCheck(original_check, input_api, output_api):
    _apply_blocklist_to_AffectedFiles(input_api)
    _modify_canned_checks(input_api.canned_checks)
    input_api.DEFAULT_FILES_TO_SKIP += _BRAVE_DEFAULT_FILES_TO_SKIP
    return original_check(input_api, output_api)


# Changes from upstream:
# 1. Add 'brave/' prefix for header guard checks to properly validate guards.
@chromium_presubmit_utils.override_check(globals())
def CheckForIncludeGuards(original_check, input_api, output_api):
    def AffectedSourceFiles(self, original_method, source_file):
        def PrependBrave(affected_file):
            affected_file = copy.copy(affected_file)
            affected_file._path = f'brave/{affected_file._path}'
            return affected_file

        return [
            PrependBrave(f) for f in filter(self.FilterSourceFile,
                                            original_method(source_file))
        ]

    with override_utils.override_scope_function(input_api,
                                                AffectedSourceFiles):
        return original_check(input_api, output_api)


# Changes from upstream:
# 1. Remove ^(chrome|components|content|extensions) filter to cover all files
#    in the repository, because brave/ structure is slightly different.
@chromium_presubmit_utils.override_check(globals())
def CheckMPArchApiUsage(original_check, input_api, output_api):
    def AffectedFiles(self, original_method, *args, **kwargs):
        kwargs['file_filter'] = self.FilterSourceFile
        return original_method(*args, **kwargs)

    with override_utils.override_scope_function(input_api, AffectedFiles):
        return original_check(input_api, output_api)


@chromium_presubmit_utils.override_check(globals())
def CheckNewHeaderWithoutGnChangeOnUpload(original_check, input_api,
                                          output_api):
    with chromium_presubmit_utils.force_presubmit_error(output_api):
        return original_check(input_api, output_api)


# We don't use OWNERS files.
@chromium_presubmit_utils.override_check(globals())
def CheckSecurityOwners(*_, **__):
    return []


# This validates added strings with screenshot tests which we don't use.
@chromium_presubmit_utils.override_check(globals())
def CheckStrings(*_, **__):
    return []


# Don't check upstream pydeps.
@chromium_presubmit_utils.override_check(globals())
def CheckPydepsNeedsUpdating(*_, **__):
    return []


# Disable Google-specific check for product images.
@chromium_presubmit_utils.override_check(globals())
def CheckNoProductIconsAddedToPublicRepo(*_, **__):
    return []


# Disable Google-specific check for support URLs.
@chromium_presubmit_utils.override_check(globals())
def CheckGoogleSupportAnswerUrlOnUpload(*_, **__):
    return []


# Disable Google-specific check for hardcoded Google API urls.
@chromium_presubmit_utils.override_check(globals())
def CheckHardcodedGoogleHostsInLowerLayers(*_, **__):
    return []
