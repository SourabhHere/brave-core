/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import styled from 'styled-components'

export const root = styled.div`
  height: 100%;
  width: 100%;
  font-family: var(--brave-font-heading);
  text-align: center;

  .icon {
    height: 80px;
    margin-bottom: 32px;
  }

  a {
    color: var(--brave-color-brandBat);
    text-decoration: none;
  }

  @media (max-width: 500px) {
    border-radius: 8px;
    padding-bottom: 99px;
  }
`

export const heading = styled.div`
  font-weight: 500;
  font-size: 16px;
  line-height: 24px;
  margin-bottom: 8px;
`

export const headingNtp = styled.div`
  font-weight: 500;
  font-size: 12px;
  line-height: 18px;
  margin-bottom: 8px;
`

export const content = styled.div`
  font-weight: 400;
  font-size: 14px;
  line-height: 20px;
  max-width: 325px;
  margin-left: auto;
  margin-right: auto;
`

export const contentNtp = styled.div`
  font-weight: 400;
  font-size: 12px;
  line-height: 18px;
  max-width: 222px;
  margin-left: auto;
  margin-right: auto;
`

export const spacing = styled.div`
  margin-top: 64px;
`

export const spacingNtp = styled.div`
  margin-top: 24px;
`

export const text = styled.div`
  margin-bottom: 8px;
`
