/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import styled from 'styled-components'

export const root = styled.div`
  height: 100%;
  width: 100%;
  background-color: var(--brave-palette-white);
  background-repeat: no-repeat;
  background-position: 0 0;
  background-size: auto 220px;
  border-radius: 16px;
  font-family: var(--brave-font-heading);
  text-align: center;
  padding: 64px 16px 313px 16px;

  .icon {
    height: 80px;
    margin-bottom: 32px;
  }

  @media (max-width: 500px) {
    border-radius: 4px;
    padding-bottom: 99px;
  }
`

export const heading = styled.div`
  font-weight: 500;
  font-size: 16px;
  line-height: 24px;
  margin-bottom: 8px;
`

export const content = styled.div`
  font-weight: 400;
  font-size: 14px;
  line-height: 20px;
  max-width: 325px;
  margin-left: auto;
  margin-right: auto;

  a {
    color: var(--brave-color-brandBat);
    text-decoration: none;
  }
`

export const spacing = styled.div`
  margin-top: 64px;
`

export const text = styled.div`
  margin-bottom: 8px;
`
