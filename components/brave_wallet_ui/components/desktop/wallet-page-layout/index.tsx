import * as React from 'react'
import { StyledWrapper, StyledContent } from './style'

export interface Props {
  isSwap?: boolean
  children?: React.ReactNode
}

export default class WalletPageLayout extends React.PureComponent<Props, {}> {
  render () {
    const { children, isSwap } = this.props
    return (
      <StyledWrapper isSwap={isSwap}>
        <StyledContent isSwap={isSwap}>
          {children}
        </StyledContent>
      </StyledWrapper>
    )
  }
}
