import styled from 'styled-components'

export const StyledWrapper = styled('div') <{ isSwap?: boolean }>`
  display: flex;
  align-items: flex-start;
  justify-content: center;
  background-color: ${(p) => p.theme.color.background02};
  width: 100%;
  min-width: ${(p) => p.isSwap ? 'unset' : '500px'};
 `

export const StyledContent = styled('div') <{ isSwap?: boolean }>`
  display: flex;
  align-items: flex-start;
  justify-content: flex-start;
  flex-direction: row;
  width: 100%;
  max-width: ${(p) => p.isSwap ? 'unset' : '1600px'};
  padding: ${(p) => p.isSwap ? '0px' : '32px 32px 0px 32px'};
  @media screen and (max-width: 800px) {
    flex-direction: column;
    align-items: center;
    padding: ${(p) => p.isSwap ? '0px' : '32px 0px 0px 0px'};
  }
 `
