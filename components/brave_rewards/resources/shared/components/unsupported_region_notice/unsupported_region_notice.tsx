/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as React from 'react'

import { LocaleContext, formatMessage } from '../../lib/locale_context'
import { NewTabLink } from '../new_tab_link'
import { BatCrossedIcon } from '../icons/bat_crossed_icon'

import * as style from './unsupported_region_notice.style'

interface Props {
  ntpCard: boolean
}

export function UnsupportedRegionNotice (props: Props) {
  const { getString } = React.useContext(LocaleContext)

  function GetHeading (ntpCard: boolean) {
    if (ntpCard) {
      return (
        <style.headingNtp>
          {getString('unsupportedRegionNoticeHeader')}
        </style.headingNtp>
      )
    } else {
      return (
        <style.heading>
          {getString('unsupportedRegionNoticeHeader')}
        </style.heading>
      )
    }
  }

  function GetLink () {
    return (
      formatMessage(getString('unsupportedRegionNoticeLearnMore'), {
        tags: {
          $1: (content) => (
            <NewTabLink key='learn' href='https://support.brave.com/hc/en-us/articles/9053832354957'>
              {content}
            </NewTabLink>
          )
        }
      })
    )
  }

  function GetText () {
    return (
      <div>
        <style.text>
          {getString('unsupportedRegionNoticeText1')}
        </style.text>
        <style.text>
          {getString('unsupportedRegionNoticeText2')}
        </style.text>
      </div>
    )
  }

  function GetContent (ntpCard: boolean) {
    if (ntpCard) {
      return (
        <style.contentNtp>
          <style.text>
            {getString('unsupportedRegionNoticeSubheader')}&nbsp;{GetLink()}
          </style.text>
          <style.spacingNtp />
          {GetText()}
        </style.contentNtp>
      )
    } else {
      return (
        <style.content>
          <style.text>
            {getString('unsupportedRegionNoticeSubheader')}
          </style.text>
          <style.text>
            {GetLink()}
          </style.text>
          <style.spacing />
          {GetText()}
        </style.content>
      )
    }
  }

  return (
    <style.root>
      {!props.ntpCard && <BatCrossedIcon />}
      {GetHeading(props.ntpCard)}
      {GetContent(props.ntpCard)}
    </style.root>
  )
}
