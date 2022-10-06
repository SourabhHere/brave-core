import * as React from 'react'
import { StyledWrapper } from './style'

export interface Props {
  isSwap?: boolean
  children?: React.ReactNode
}

export default class WalletSubViewLayout extends React.PureComponent<Props, {}> {
  render () {
    const { children, isSwap } = this.props
    return (
      <StyledWrapper isSwap={isSwap}>
        {children}
      </StyledWrapper>
    )
  }
}
